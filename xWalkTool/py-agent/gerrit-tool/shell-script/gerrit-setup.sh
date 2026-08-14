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
    server_host="${GERRIT_SERVER_HOST:-${GERRIT_SERVER_IP:-${EDUVPN_SERVER_IP:-}}}"
    case "$server_host" in
        ""|SERVER_IP_FROM_ASSESSMENT|LOCAL_LINUX_IP_FROM_ASSESSMENT)
            echo "Set the server IP in $conf" >&2
            exit 2
            ;;
    esac
    [ "$GERRIT_SHA256" != "OFFICIAL_GERRIT_WAR_SHA256" ] || {
        echo "Set GERRIT_SHA256 in $conf" >&2
        exit 2
    }
    storage_path="${GERRIT_STORAGE_PATH:-}"
    http_port="${GERRIT_HTTP_PORT:-8080}"
    ssh_port="${GERRIT_SSH_PORT:-29418}"
    https_port="${GERRIT_HTTPS_PORT:-18443}"
    http_listen_url="${GERRIT_HTTP_LISTEN_URL:-proxy-https://127.0.0.1:${http_port}/}"
    canonical_web_url="${GERRIT_CANONICAL_WEB_URL:-https://${server_host}:${https_port}/}"
    ssh_listen_address="${GERRIT_SSH_LISTEN_ADDRESS:-127.0.0.1:${ssh_port}}"
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
        --server-host "$server_host" \
        --storage-path "$storage_path" \
        --gerrit-url "$GERRIT_URL" \
        --gerrit-sha256 "$GERRIT_SHA256" \
        --admin-username "$GERRIT_ADMIN_USER" \
        --admin-full-name "$GERRIT_ADMIN_NAME" \
        --admin-role "$GERRIT_ADMIN_ROLE" \
        --admin-email "$GERRIT_ADMIN_EMAIL" \
        --project-name "$GERRIT_PROJECT" \
        --project-branch "$GERRIT_BRANCH" \
        --https-port "$https_port" \
        --ssh-port "$ssh_port" \
        --http-port "$http_port" \
        --http-listen-url "$http_listen_url" \
        --canonical-web-url "$canonical_web_url" \
        --ssh-listen-address "$ssh_listen_address" \
        --process-manager "$GERRIT_PROCESS_MANAGER"
    unset GERRIT_ADMIN_PASSWORD
    start
}

usage()
{
    echo "Usage: gerrit-setup.sh assess|validate-storage|install|start" >&2
    exit 2
}

[ "$#" -ge 1 ] || usage
case "$1" in
    assess) [ "$#" -eq 1 ] || usage; need_inst; python3 "$installer" assess ;;
    validate-storage)
        [ "$#" -eq 1 ] || usage
        need_inst
        [ -r "$conf" ] || { echo "Missing Gerrit configuration: $conf" >&2; exit 2; }
        # shellcheck source=/dev/null
        . "$conf"
        python3 "$installer" validate-storage --storage-path "${GERRIT_STORAGE_PATH:-}"
        ;;
    install) [ "$#" -eq 1 ] || usage; install ;;
    start) [ "$#" -eq 1 ] || usage; start ;;
    *) usage ;;
esac
