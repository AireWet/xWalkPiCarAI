#!/usr/bin/env bash
set -eu

dir="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)"
root="$(CDPATH='' cd -- "$dir/.." && pwd -P)"
installer="$root/py-src/xWalkGerritServerSetup.py"
conf="${XWALK_GERRIT_SETUP_CONFIG:-$root/config/gerrit-setup.conf}"

need_inst()
{
    [ -r "$installer" ] || {
        echo "Missing Gerrit Python installer: $installer" >&2
        exit 2
    }
    command -v python3 >/dev/null 2>&1 || {
        echo "python3 is required" >&2
        exit 2
    }
}

need_site()
{
    [ -x "$HOME/bin/gerrit-start" ] || {
        echo "Gerrit is not installed; run gerrit-setup.sh install first" >&2
        exit 2
    }
    [ -x "$HOME/bin/gerrit-status" ] || {
        echo "Missing installed Gerrit status command" >&2
        exit 2
    }
    [ -x "$HOME/bin/gerrit-check" ] || {
        echo "Missing installed Gerrit validation command" >&2
        exit 2
    }
}

load_conf()
{
    [ -r "$conf" ] || { echo "Missing Gerrit configuration: $conf" >&2; exit 2; }
    # shellcheck source=/dev/null
    . "$conf"
    server_ip="${GERRIT_SERVER_IP:-${EDUVPN_SERVER_IP:-}}"
    case "$server_ip" in
        ""|SERVER_IP_FROM_ASSESSMENT|LOCAL_LINUX_IP_FROM_ASSESSMENT)
            echo "Set the server IP in $conf" >&2
            exit 2
            ;;
    esac
    [ "$GERRIT_SHA256" != "OFFICIAL_GERRIT_WAR_SHA256" ] || {
        echo "Set GERRIT_SHA256 in $conf" >&2
        exit 2
    }
}

read_pass()
{
    if [ -z "${GERRIT_ADMIN_PASSWORD:-}" ]; then
        read -r -s -p "Initial $GERRIT_ADMIN_USER web password: " GERRIT_ADMIN_PASSWORD
        printf '\n'
    fi
    export GERRIT_ADMIN_PASSWORD
}

start()
{
    need_site
    if ! "$HOME/bin/gerrit-status" >/dev/null 2>&1; then
        "$HOME/bin/gerrit-start"
    else
        # shellcheck source=/dev/null
        . "$HOME/gerrit-env.sh"
        if [ "${GERRIT_PROXY_MODE:-}" = "local-http" ] &&
            ! "$HOME/bin/gerrit-caddy-control" status >/dev/null 2>&1
        then
            "$HOME/bin/gerrit-caddy-control" start
        fi
    fi
    "$HOME/bin/gerrit-check"
}

install()
{
    need_inst
    load_conf
    read_pass
    python3 "$installer" install \
        --server-ip "$server_ip" \
        --gerrit-url "$GERRIT_URL" \
        --gerrit-sha256 "$GERRIT_SHA256" \
        --admin-username "$GERRIT_ADMIN_USER" \
        --admin-full-name "$GERRIT_ADMIN_NAME" \
        --admin-role "$GERRIT_ADMIN_ROLE" \
        --admin-email "$GERRIT_ADMIN_EMAIL" \
        --project-name "$GERRIT_PROJECT" \
        --project-branch "$GERRIT_BRANCH" \
        --https-port "$GERRIT_HTTPS_PORT" \
        --ssh-port "$GERRIT_SSH_PORT" \
        --init-http-port "$GERRIT_HTTP_PORT" \
        --process-manager "$GERRIT_PROCESS_MANAGER"
    unset GERRIT_ADMIN_PASSWORD
    start
}

usage()
{
    echo "Usage: gerrit-setup.sh assess|install|start" >&2
    exit 2
}

[ "$#" -ge 1 ] || usage
case "$1" in
    assess) [ "$#" -eq 1 ] || usage; need_inst; python3 "$installer" assess ;;
    install) [ "$#" -eq 1 ] || usage; install ;;
    start) [ "$#" -eq 1 ] || usage; start ;;
    *) usage ;;
esac
