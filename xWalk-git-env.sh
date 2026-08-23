#!/usr/bin/env bash

# Source this file from Bash to load the non-secret xWalk Git environment.
# Machine-specific overrides belong in $HOME/.config/xwalk/git-env.local.sh.

if [[ "${BASH_SOURCE[0]}" == "$0" ]]
then
    printf 'Source this file instead of executing it: source %s\n' "$0" >&2
    exit 2
fi

_xwalk_git_env_script_directory=$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if ! _xwalk_git_env_repository_root=$(git -C "${_xwalk_git_env_script_directory}" rev-parse --show-toplevel 2>/dev/null)
then
    printf 'Unable to resolve the xWalk Git repository from %s\n' "${_xwalk_git_env_script_directory}" >&2
    return 2
fi

export XWALK_REPOSITORY_ROOT="${_xwalk_git_env_repository_root}"
export XWALK_GIT_ENV_FILE="${XWALK_REPOSITORY_ROOT}/xWalk-git-env.sh"
export XWALK_GERRIT_SERVER_ENV_FILE="${XWALK_GERRIT_SERVER_ENV_FILE:-${HOME}/gerrit-env.sh}"
export XWALK_GIT_ENV_LOCAL_FILE="${XWALK_GIT_ENV_LOCAL_FILE:-${HOME}/.config/xwalk/git-env.local.sh}"
export XWALK_CI_ENV_FILE="${XWALK_CI_ENV_FILE:-${HOME}/.xwalk-ci.env}"

# Import non-secret server installation paths when this is the Gerrit host.
if [[ -r "${XWALK_GERRIT_SERVER_ENV_FILE}" ]]
then
    # shellcheck source=/dev/null
    source "${XWALK_GERRIT_SERVER_ENV_FILE}"
fi

# Public defaults for the current xWalk Gerrit and GitHub deployment.
: "${GERRIT_SERVER_HOST:=${GERRIT_HOST:-192.168.1.158}}"
: "${GERRIT_HOST:=${GERRIT_SERVER_HOST}}"
: "${GERRIT_SSH_PORT:=29419}"
: "${GERRIT_HTTPS_PORT:=18443}"
: "${GERRIT_HTTP_PORT:=18080}"
: "${GERRIT_ADMIN_USER:=joxy}"
: "${GERRIT_USER:=${GERRIT_ADMIN_USER}}"
: "${GERRIT_CI_USERNAME:=xwalk-ci}"
: "${GERRIT_PROJECT:=xWalkPiCarAI}"
: "${GERRIT_BRANCH:=master}"
: "${GERRIT_INTEGRATION_PROJECT:=${GERRIT_PROJECT}}"
: "${GERRIT_INTEGRATION_BRANCH:=${GERRIT_BRANCH}}"
: "${GERRIT_SITE:=${HOME}/gerrit-site}"
: "${GERRIT_PROCESS_MANAGER:=systemd}"
: "${GERRIT_UPLIFT_ENABLED:=false}"
: "${GERRIT_UPLIFT_AUTO_SUBMIT:=false}"
: "${GERRIT_UPLIFT_AUTO_REVIEW:=false}"
: "${XWALK_GIT_AUTO_START:=true}"
: "${GITHUB_REPOSITORY_OWNER:=jochuuu}"
: "${GITHUB_REPOSITORY_NAME:=xWalkPiCarAI}"
: "${GITHUB_SYNC_SOURCE_PROJECT:=${GERRIT_INTEGRATION_PROJECT}}"
: "${GITHUB_SYNC_SOURCE_BRANCH:=${GERRIT_INTEGRATION_BRANCH}}"
: "${GITHUB_INTEGRATION_BRANCH:=${GITHUB_SYNC_SOURCE_BRANCH}}"
: "${GITHUB_PUSH_ENABLED:=false}"

# Apply personal non-secret overrides after the portable defaults. Never place
# passwords, access tokens, private keys, cookies, or authenticated URLs here.
if [[ -r "${XWALK_GIT_ENV_LOCAL_FILE}" ]]
then
    # shellcheck source=/dev/null
    source "${XWALK_GIT_ENV_LOCAL_FILE}"
fi

GERRIT_HOST="${GERRIT_SERVER_HOST}"
export GERRIT_SERVER_HOST GERRIT_HOST GERRIT_SSH_PORT GERRIT_HTTPS_PORT GERRIT_HTTP_PORT
export GERRIT_ADMIN_USER GERRIT_USER GERRIT_CI_USERNAME GERRIT_PROJECT GERRIT_BRANCH
export GERRIT_INTEGRATION_PROJECT GERRIT_INTEGRATION_BRANCH GERRIT_PROCESS_MANAGER GERRIT_SITE
export GERRIT_UPLIFT_ENABLED GERRIT_UPLIFT_AUTO_SUBMIT GERRIT_UPLIFT_AUTO_REVIEW
export XWALK_GIT_AUTO_START
export GITHUB_REPOSITORY_OWNER GITHUB_REPOSITORY_NAME GITHUB_SYNC_SOURCE_PROJECT
export GITHUB_SYNC_SOURCE_BRANCH GITHUB_INTEGRATION_BRANCH GITHUB_PUSH_ENABLED

export GERRIT_BASE_URL="ssh://${GERRIT_SERVER_HOST}:${GERRIT_SSH_PORT}"
export GERRIT_SSH_URL="ssh://${GERRIT_USER}@${GERRIT_SERVER_HOST}:${GERRIT_SSH_PORT}"
export GERRIT_CANONICAL_WEB_URL="https://${GERRIT_SERVER_HOST}:${GERRIT_HTTPS_PORT}/"
export GERRIT_WEB_URL="${GERRIT_CANONICAL_WEB_URL%/}"
export GERRIT_HTTP_BASE_URL="${GERRIT_WEB_URL}"
export GERRIT_SSH_LISTEN_ADDRESS="${GERRIT_SERVER_HOST}:${GERRIT_SSH_PORT}"
export GERRIT_HTTP_LISTEN_URL="proxy-https://127.0.0.1:${GERRIT_HTTP_PORT}/"
export XWALK_GERRIT_PROJECT_URL="${GERRIT_WEB_URL}/q/project:${GERRIT_PROJECT}"
export XWALK_GERRIT_REMOTE_URL="${GERRIT_SSH_URL}/${GERRIT_PROJECT}"
export XWALK_GERRIT_REVIEW_REF="refs/for/${GERRIT_BRANCH}"
export XWALK_GERRIT_WIP_REVIEW_REF="refs/for/${GERRIT_BRANCH}%wip"

export GITHUB_REPOSITORY="${GITHUB_REPOSITORY_OWNER}/${GITHUB_REPOSITORY_NAME}"
export GITHUB_INTEGRATION_WEB_URL="https://github.com/${GITHUB_REPOSITORY}"
export GITHUB_INTEGRATION_REMOTE="git@github.com:${GITHUB_REPOSITORY}.git"
export XWALK_DEVELOPER_NOTE_URL="https://${GITHUB_REPOSITORY_OWNER}.github.io/${GITHUB_REPOSITORY_NAME}/"

export XWALK_GERRIT_COMPONENT_REPOSITORIES="DevloperNote xWalkAgent xWalkAudioResources xWalkController xWalkHal xWalk-rpi5-iw xWalkLibrary xWalk-rpi5-tool xWalkTrace"
export GERRIT_VERIFICATION_TARGETS="${GERRIT_VERIFICATION_TARGETS:-xWalkPiCarAI:master,DevloperNote:master,xWalkAgent:master,xWalkAudioResources:master,xWalkController:master,xWalkHal:master,xWalk-rpi5-iw:master,xWalkLibrary:master,xWalk-rpi5-tool:master,xWalkTrace:master}"

export XWALK_TOOL_DIRECTORY="${XWALK_REPOSITORY_ROOT}/xWalk-rpi5-tool"
export XWALK_RPI5_DIRECTORY="${XWALK_REPOSITORY_ROOT}/xWalk-rpi5-hw"
export XWALK_DEVELOPER_NOTE_DIRECTORY="${XWALK_REPOSITORY_ROOT}/devloper-note"
export XWALK_GERRIT_TOOL_DIRECTORY="${XWALK_TOOL_DIRECTORY}/py-agent/gerrit-tool"
export XWALK_GERRIT_GIT_CONNECT="${XWALK_GERRIT_TOOL_DIRECTORY}/bin/xwalk-gerrit-git-connect"
export XWALK_DOC_TOOL_DIRECTORY="${XWALK_TOOL_DIRECTORY}/doc-tool"
export XWALK_GERRIT_TOOL_HOME="${XWALK_GERRIT_TOOL_HOME:-${XWALK_GERRIT_TOOL_DIRECTORY}/py-src}"
export XWALK_CI_STATE_DIRECTORY="${XWALK_CI_STATE_DIRECTORY:-${HOME}/gerrit-ci/state}"
export XWALK_CI_LOG_DIRECTORY="${XWALK_CI_LOG_DIRECTORY:-${HOME}/gerrit-ci/logs}"
export XWALK_CI_WORK_DIRECTORY="${XWALK_CI_WORK_DIRECTORY:-${HOME}/gerrit-ci/work}"
export XWALK_CI_LOG_HTTP_HOST="${XWALK_CI_LOG_HTTP_HOST:-127.0.0.1}"
export XWALK_CI_LOG_HTTP_PORT="${XWALK_CI_LOG_HTTP_PORT:-8091}"
export XWALK_CI_LOG_WEB_URL="${XWALK_CI_LOG_WEB_URL:-${GERRIT_WEB_URL}/ci}"

export XWALK_CONFIGURED_SUBMODULE_PATHS
XWALK_CONFIGURED_SUBMODULE_PATHS=$(git -C "${XWALK_REPOSITORY_ROOT}" config \
    --file .gitmodules --get-regexp '^submodule\..*\.path$' 2>/dev/null | awk '{print $2}' | xargs)
export XWALK_GITLINK_SUBMODULE_PATHS
XWALK_GITLINK_SUBMODULE_PATHS=$(git -C "${XWALK_REPOSITORY_ROOT}" ls-files --stage | \
    awk '$1 == "160000" {print $4}' | xargs)
export XWALK_GIT_USER_NAME
XWALK_GIT_USER_NAME=$(git -C "${XWALK_REPOSITORY_ROOT}" config user.name 2>/dev/null || true)
export XWALK_GIT_USER_EMAIL
XWALK_GIT_USER_EMAIL=$(git -C "${XWALK_REPOSITORY_ROOT}" config user.email 2>/dev/null || true)

_xwalk_git_env_prepend_path()
{
    local candidate=$1

    [[ -d "${candidate}" ]] || return 0
    case ":${PATH}:" in
        *":${candidate}:"*) ;;
        *) PATH="${candidate}:${PATH}" ;;
    esac
}

_xwalk_git_env_prepend_path "${HOME}/bin"
_xwalk_git_env_prepend_path "${XWALK_GERRIT_TOOL_DIRECTORY}/bin"
export PATH
unset -f _xwalk_git_env_prepend_path

_xwalk_git_env_block_direct_github_pushes()
{
    local repository_path remote_name push_urls
    local blocked_url="xwalk-gerrit-uplift-only://direct-github-push-disabled"

    for repository_path in "${XWALK_REPOSITORY_ROOT}" ${XWALK_GITLINK_SUBMODULE_PATHS}
    do
        [[ "${repository_path}" == "${XWALK_REPOSITORY_ROOT}" ]] || \
            repository_path="${XWALK_REPOSITORY_ROOT}/${repository_path}"
        [[ -d "${repository_path}" ]] || continue
        while IFS= read -r remote_name
        do
            [[ -n "${remote_name}" ]] || continue
            push_urls=$(git -C "${repository_path}" remote get-url \
                --push --all "${remote_name}" 2>/dev/null || true)
            if grep -qE '(^|[@/:])github\.com([/:]|$)' <<<"${push_urls}"
            then
                if ! git -C "${repository_path}" config --local --replace-all \
                    "remote.${remote_name}.pushurl" "${blocked_url}"
                then
                    printf 'Unable to guard GitHub push URL for %s/%s.\n' \
                        "${repository_path}" "${remote_name}" >&2
                    return 1
                fi
            fi
        done < <(git -C "${repository_path}" remote)
    done
}

_xwalk_git_env_project_name()
{
    local repository_path=$1

    if [[ "${repository_path}" == "${XWALK_REPOSITORY_ROOT}" ]]
    then
        printf '%s\n' "${GERRIT_PROJECT}"
    elif [[ "${repository_path}" == "${XWALK_DEVELOPER_NOTE_DIRECTORY}" ]]
    then
        printf 'DevloperNote\n'
    else
        basename -- "${repository_path}"
    fi
}

_xwalk_git_env_configure_gerrit_pushes()
{
    local repository_path remote_name project_name push_url

    [[ -x "${XWALK_GERRIT_GIT_CONNECT}" ]] || {
        printf 'Missing Gerrit Git connector: %s\n' "${XWALK_GERRIT_GIT_CONNECT}" >&2
        return 1
    }
    [[ "${XWALK_GERRIT_GIT_CONNECT}" != *[[:space:]]* ]] || {
        printf 'Gerrit Git connector path must not contain whitespace: %s\n' \
            "${XWALK_GERRIT_GIT_CONNECT}" >&2
        return 1
    }

    for repository_path in "${XWALK_REPOSITORY_ROOT}" ${XWALK_GITLINK_SUBMODULE_PATHS}
    do
        [[ "${repository_path}" == "${XWALK_REPOSITORY_ROOT}" ]] || \
            repository_path="${XWALK_REPOSITORY_ROOT}/${repository_path}"
        [[ -d "${repository_path}" ]] || continue
        project_name=$(_xwalk_git_env_project_name "${repository_path}")
        push_url="ext::${XWALK_GERRIT_GIT_CONNECT} ${GERRIT_USER}@${GERRIT_SERVER_HOST} "
        push_url+="${GERRIT_SSH_PORT} ${project_name} ${XWALK_GIT_AUTO_START} %S"
        git -C "${repository_path}" config --local protocol.ext.allow always
        for remote_name in origin gerrit
        do
            if git -C "${repository_path}" config --local --get "remote.${remote_name}.url" >/dev/null
            then
                git -C "${repository_path}" config --local --replace-all \
                    "remote.${remote_name}.pushurl" "${push_url}"
            fi
        done
    done
}

if ! _xwalk_git_env_block_direct_github_pushes
then
    unset -f _xwalk_git_env_block_direct_github_pushes
    return 1
fi
unset -f _xwalk_git_env_block_direct_github_pushes

xwalk_git_env_show()
{
    printf '%-30s %s\n' \
        'Repository root:' "${XWALK_REPOSITORY_ROOT}" \
        'Git identity:' "${XWALK_GIT_USER_NAME:-unset} <${XWALK_GIT_USER_EMAIL:-unset}>" \
        'Gerrit project:' "${GERRIT_PROJECT}/${GERRIT_BRANCH}" \
        'Gerrit SSH:' "${GERRIT_USER}@${GERRIT_SERVER_HOST}:${GERRIT_SSH_PORT}" \
        'Gerrit web:' "${GERRIT_WEB_URL}" \
        'Gerrit CI account:' "${GERRIT_CI_USERNAME}" \
        'Gerrit CI dashboard:' "${XWALK_CI_LOG_WEB_URL}" \
        'Gerrit CI state:' "${XWALK_CI_STATE_DIRECTORY}" \
        'Automatic server start:' "${XWALK_GIT_AUTO_START}" \
        'Source publication:' 'Gerrit review and guarded uplift only' \
        'GitHub repository:' "${GITHUB_REPOSITORY}" \
        'Developer notes:' "${XWALK_DEVELOPER_NOTE_URL}" \
        'Configured submodules:' "${XWALK_CONFIGURED_SUBMODULE_PATHS:-none}" \
        'Actual gitlinks:' "${XWALK_GITLINK_SUBMODULE_PATHS:-none}" \
        'Private CI env:' "${XWALK_CI_ENV_FILE} (not sourced)" \
        'Local override:' "${XWALK_GIT_ENV_LOCAL_FILE}"
}

xwalk_git_env_help()
{
    cat <<'EOF'
xWalk Git environment commands:
  xwalk_git_env_show                 Show loaded non-secret configuration.
  xwalk_git_modules                  Show configured modules and actual gitlinks.
  xwalk_git_status                   Show root and real-submodule Git status.
  xwalk_git_start_servers            Start Gerrit, HTTPS proxy, and Gerrit CI.
  xwalk_gerrit_push [PATH] [BRANCH] [REMOTE]
                                     Upload HEAD for active Gerrit review.
  xwalk_gerrit_push_wip [PATH] [BRANCH] [REMOTE]
                                     Upload HEAD as a Gerrit WIP review.

Defaults:
  PATH     repository root
  BRANCH   branch tracked by the current checkout, otherwise GERRIT_BRANCH
  REMOTE   origin

Examples:
  xwalk_git_start_servers
  xwalk_gerrit_push
EOF
}

xwalk_git_start_servers()
{
    local start_command="${HOME}/bin/gerrit-start"

    if [[ ! -x "${start_command}" ]]
    then
        printf 'Gerrit server is not installed on this machine; auto-start skipped.\n'
        return 0
    fi
    if [[ ! -d "${GERRIT_SITE}" ]]
    then
        printf 'Gerrit site is unavailable at %s; auto-start skipped.\n' "${GERRIT_SITE}"
        return 0
    fi
    "${start_command}"
}

xwalk_git_modules()
{
    printf 'Configured module paths (.gitmodules):\n'
    git -C "${XWALK_REPOSITORY_ROOT}" config --file .gitmodules \
        --get-regexp '^submodule\..*\.(path|url|branch)$'
    printf '\nActual gitlink submodules (index mode 160000):\n'
    git -C "${XWALK_REPOSITORY_ROOT}" submodule status
}

xwalk_git_status()
{
    local module_path

    printf '\n[%s]\n' "${XWALK_REPOSITORY_ROOT}"
    git -C "${XWALK_REPOSITORY_ROOT}" status --short --branch
    for module_path in ${XWALK_GITLINK_SUBMODULE_PATHS}
    do
        printf '\n[%s]\n' "${module_path}"
        git -C "${XWALK_REPOSITORY_ROOT}/${module_path}" status --short --branch
    done
}

_xwalk_gerrit_target_branch()
{
    local repository_path=$1
    local upstream

    upstream=$(git -C "${repository_path}" rev-parse --abbrev-ref '@{upstream}' 2>/dev/null || true)
    if [[ -n "${upstream}" ]]
    then
        printf '%s\n' "${upstream#*/}"
    else
        printf '%s\n' "${GERRIT_BRANCH}"
    fi
}

xwalk_gerrit_push()
{
    local repository_path=${1:-${XWALK_REPOSITORY_ROOT}}
    local target_branch=${2:-}
    local remote_name=${3:-origin}

    [[ -n "${target_branch}" ]] || target_branch=$(_xwalk_gerrit_target_branch "${repository_path}")
    git -C "${repository_path}" push "${remote_name}" "HEAD:refs/for/${target_branch}"
}

xwalk_gerrit_push_wip()
{
    local repository_path=${1:-${XWALK_REPOSITORY_ROOT}}
    local target_branch=${2:-}
    local remote_name=${3:-origin}

    [[ -n "${target_branch}" ]] || target_branch=$(_xwalk_gerrit_target_branch "${repository_path}")
    git -C "${repository_path}" push "${remote_name}" "HEAD:refs/for/${target_branch}%wip"
}

if [[ "${XWALK_GIT_AUTO_START}" != true && "${XWALK_GIT_AUTO_START}" != false ]]
then
    printf 'XWALK_GIT_AUTO_START must be true or false.\n' >&2
    return 2
fi
if ! _xwalk_git_env_configure_gerrit_pushes
then
    unset -f _xwalk_git_env_configure_gerrit_pushes _xwalk_git_env_project_name
    return 1
fi
unset -f _xwalk_git_env_configure_gerrit_pushes _xwalk_git_env_project_name

if [[ "${XWALK_GIT_ENV_QUIET:-false}" != true ]]
then
    printf 'xWalk Git environment loaded: %s\n' "${XWALK_REPOSITORY_ROOT}"
    printf 'Run xwalk_git_env_help for commands or xwalk_git_env_show for configuration.\n'
fi

unset _xwalk_git_env_repository_root _xwalk_git_env_script_directory
