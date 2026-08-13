# Gerrit security and remote access

## HTTPS and certificate trust

The browser endpoint is `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`. Distribute
only `$HOME/gerrit-site/etc/gerrit-self-signed.crt` through an authenticated
channel and compare its SHA-256 fingerprint out of band:
`@@CERTIFICATE_FINGERPRINT@@`. Never distribute the private key, expose a
password, or disable certificate validation.

The certificate is intended for controlled university-network use. Replace it
before expiry using a reviewed procedure for the same confirmed server address.

## Anonymous review access

Anyone who can reach the server through the university network or eduVPN may
view changes when `Anonymous Users` has Read permission on `refs/heads/*`:

```text
https://@@SERVER_IP@@:@@HTTPS_PORT@@/q/project:@@PROJECT_NAME@@
```

Anonymous users must not receive push, label, submit, owner,
create-reference, or administrative permission. Caddy blocks Git-over-HTTP
protocol endpoints. Clone, fetch, push, voting, and submission require an
individual Gerrit account and registered public SSH key.

## Remote validation

From a different computer connected through eduVPN, run:

```bash
nc -vz @@SERVER_IP@@ @@HTTPS_PORT@@
```

```bash
nc -vz @@SERVER_IP@@ @@SSH_PORT@@
```

```bash
curl -v https://@@SERVER_IP@@:@@HTTPS_PORT@@/
```

```bash
ssh -v -p @@SSH_PORT@@ USERNAME@@@SERVER_IP@@
```

Verify public viewing, certificate trust, individual login, SSH clone, upload
to `refs/for/@@PROJECT_BRANCH@@`, review attribution, and rejection of direct
push to `refs/heads/@@PROJECT_BRANCH@@`.

If the ports remain unreachable despite correct local binding, request college
IT to permit TCP @@HTTPS_PORT@@ and @@SSH_PORT@@ to `@@SERVER_IP@@` only from
approved university or eduVPN subnets. Do not bypass routing or firewall rules.
