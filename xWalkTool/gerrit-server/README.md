# Non-root Gerrit server installer

`xWalkGerritServerSetup.py` installs a Gerrit 3.14 server entirely below the
current user's home directory. Copy this directory to the college server, or
clone the repository there, and run the assessment first:

```bash
python3 xWalkTool/gerrit-server/xWalkGerritServerSetup.py assess
```

The installer never invokes `sudo`, a system package manager, Docker, a
firewall tool, or a system service manager. Installation requires values that
cannot be safely discovered or invented:

- the IP address assigned to the college server interface reachable through
  eduVPN;
- an approved LDAPS endpoint and directory schema;
- the SHA-256 checksum published for the selected Gerrit WAR;
- either an approved PKCS12 TLS keystore or explicit acceptance of an
  internal-only self-signed certificate;
- a keystore password supplied through `GERRIT_KEYSTORE_PASSWORD`.

## Find the college-accessible web address

Run the assessment on the actual college server, not on a developer laptop:

```bash
python3 xWalkTool/gerrit-server/xWalkGerritServerSetup.py assess
```

In the reported `addresses`, select only the address assigned to the physical
college-server interface that college IT confirms is reachable from eduVPN.
Do not select loopback, link-local, Docker, container, virtual-machine-only, or
guessed addresses. Pass the confirmed address to `install` with `--server-ip`.

After installation, print the exact address that users must open:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

The expected form is `https://COLLEGE_SERVER_IP:18443/`. Although this is
commonly called the Gerrit HTTP or web address, remote users must use HTTPS.
The temporary `http://127.0.0.1:18080/` initialization address is local-only
and must never be shared.

The URL for all visible changes in one project is:

```text
https://COLLEGE_SERVER_IP:18443/q/project:PROJECT_NAME
```

Every authorised college user must connect to eduVPN, trust the approved
certificate, and sign in with an individual account. In Gerrit project access,
grant **Read** only to the intended college groups. Remove inherited
**Anonymous Users: Read** access when the project must not be visible without
authentication. Use **Registered Users** only when every authenticated college
account should be able to see the project.

From a different authorised computer connected through eduVPN, verify the web
port and open the reported URL:

```bash
nc -vz COLLEGE_SERVER_IP 18443
```

If this fails while local access works, college IT must permit TCP port `18443`
from the authorised eduVPN subnet to that server. Do not claim remote access
works until this different-computer test succeeds.

Example using approved LDAP and an approved keystore. Read the password
silently so it is not included in the command history:

```bash
read -r -s -p 'Gerrit keystore password: ' GERRIT_KEYSTORE_PASSWORD
export GERRIT_KEYSTORE_PASSWORD
python3 xWalkTool/gerrit-server/xWalkGerritServerSetup.py install --server-ip 10.20.30.40 --gerrit-sha256 PUBLISHED_SHA256 --auth-type ldap --ldap-server ldaps://directory.college.example --ldap-account-base 'ou=people,dc=college,dc=example' --ldap-account-pattern '(uid=${username})' --tls-mode import --tls-keystore /secure/user/path/gerrit.p12 --tls-ca-certificate /secure/user/path/college-ca-chain.crt --project-name MyPiCarX
unset GERRIT_KEYSTORE_PASSWORD
```

Do not put the password or real identity-provider secrets in shell history,
documentation, or Git. Prefer reading the environment value from a private
secret manager or entering it in a private shell session.

For controlled testing only, `--tls-mode self-signed` creates a server
certificate containing the selected IP address as a subject alternative name.
Both users must import the exported certificate into their trust stores; they
must not disable certificate validation.

The install command performs local loopback validation before writing the
final IP-specific configuration. It creates user-owned control and backup
commands in `$HOME/bin`, plus completed server and partner guides below
`$HOME/gerrit-site/docs`. It cannot securely automate the first LDAP login,
register users' public keys, or prove eduVPN reachability from another computer.
Those actions remain documented post-install gates.

Run the host-only tests with:

```bash
python3 -m unittest xWalkTool/gerrit-server/xWalkGerritServerSetupTest.py
```
