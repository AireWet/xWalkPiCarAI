#!/usr/bin/env bash
set -Eeuo pipefail
umask 077

script_dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
# shellcheck source=xWalkTool/gerrit/shell-script/xwalk-gerrit-common.sh
source "$script_dir/xwalk-gerrit-common.sh"

[[ "$#" -eq 1 ]] || { echo "Usage: gerrit-permission-check.sh --dry-run|--apply" >&2; exit 2; }
xwalk_mode "$1"
xwalk_load_config

expectation()
{
    local identity="$1" repository="$2"
    case "$identity:$repository" in
        public:DevloperNote|public:xWalkHal|public:xWalkLibrary|public:xWalkTrace) echo read ;;
        public:*) echo deny ;;
        partner:DevloperNote) echo develop ;;
        partner:xWalkHal|partner:xWalkController|partner:xWalkLibrary|partner:xWalkTrace) echo review ;;
        partner:xWalkIW|partner:xWalkAgent) echo read ;;
        partner:*) echo deny ;;
        ci:xWalk-rpi5) echo review-verify ;;
        ci:*) echo verify ;;
        owner:*) echo owner ;;
    esac
}

identity_settings()
{
    case "$1" in
        partner) printf '%s\t%s\n' "${GERRIT_PARTNER_USERNAME:-}" "${GERRIT_PARTNER_SSH_KEY:-}" ;;
        ci) printf '%s\t%s\n' "$GERRIT_CI_USERNAME" "${GERRIT_CI_SSH_KEY:-}" ;;
        owner) printf '%s\t%s\n' "$GERRIT_ADMIN_USERNAME" "${GERRIT_OWNER_SSH_KEY:-}" ;;
    esac
}

check_authenticated_read()
{
    local identity="$1" repository="$2" expected="$3" settings username key url result status
    settings="$(identity_settings "$identity")"
    IFS=$'\t' read -r username key <<< "$settings"
    if [[ -z "$username" || -z "$key" || ! -r "$key" ]]; then
        printf '[skipped] %s %s: configure username and read-only test key\n' "$identity" "$repository"
        xwalk_log "verify-$identity-access" "validation" "$repository" "$identity" "unchecked" \
            "$expected" "Skipped effective permission verification" \
            "A dedicated $identity test identity and public-key credential are required." "Skipped" \
            "Set the documented username and key variables; never share private keys"
        return
    fi
    url="ssh://$username@$GERRIT_SERVER_HOST:$GERRIT_SSH_PORT/$repository"
    if GIT_SSH_COMMAND="ssh -i $(printf '%q' "$key") -o BatchMode=yes -o IdentitiesOnly=yes" \
        git ls-remote "$url" >/dev/null 2>&1; then
        result="read"
    else
        result="deny"
    fi
    status=Verified
    case "$expected:$result" in
        deny:deny|read:read|review:read|develop:read|verify:read|review-verify:read|owner:read) ;;
        *) status=Failed ;;
    esac
    xwalk_log "verify-$identity-access" "validation" "$repository" "$identity" "unchecked" \
        "$result" "Checked effective repository visibility" \
        "Repository visibility must match the fixed access matrix." "$status" \
        "git ls-remote returned the expected visibility result"
    [[ "$status" == Verified ]]
}

check_public_read()
{
    local repository="$1" expected="$2" result status url
    if [[ -z "${GERRIT_HTTP_BASE_URL:-}" ]]; then
        xwalk_log "verify-public-access" "validation" "$repository" "public" "unchecked" "$expected" \
            "Skipped anonymous permission verification" \
            "GERRIT_HTTP_BASE_URL is required for an unauthenticated clone check." "Skipped" \
            "Set the canonical Gerrit HTTP base URL"
        return
    fi
    url="${GERRIT_HTTP_BASE_URL%/}/$repository"
    if git -c credential.helper= ls-remote "$url" >/dev/null 2>&1; then
        result="read"
    else
        result="deny"
    fi
    status=Verified
    [[ "$result" == "$expected" ]] || status=Failed
    xwalk_log "verify-public-access" "validation" "$repository" "public" "unchecked" "$result" \
        "Checked anonymous repository visibility" \
        "Only four approved components may be public." "$status" "Unauthenticated git ls-remote completed"
    [[ "$status" == Verified ]]
}

main()
{
    local identity repository expected
    for identity in public partner ci owner; do
        for repository in "${xwalk_repositories[@]}"; do
            expected="$(expectation "$identity" "$repository")"
            if [[ "$XWALK_MODE" == dry-run ]]; then
                printf '[dry-run] verify %-7s %-20s expected=%s\n' "$identity" "$repository" "$expected"
                xwalk_log "verify-$identity-access" "validation" "$repository" "$identity" \
                    "unchecked" "$expected" "Plan effective permission verification" \
                    "Each identity and repository requires a separate access result." "Planned"
            elif [[ "$identity" == public ]]; then
                check_public_read "$repository" "$expected"
            else
                check_authenticated_read "$identity" "$repository" "$expected"
            fi
        done
    done
    echo "Read checks do not prove review, label, submit, force-push, deletion, or ACL rights."
    echo "Use an administrator account to inspect each refs/meta/config and dedicated test accounts for negative checks."
}

main
