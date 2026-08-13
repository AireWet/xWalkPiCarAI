# Gerrit partner onboarding

- Server: `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`
- SSH: `@@SERVER_IP@@:@@SSH_PORT@@`
- Project: `@@PROJECT_NAME@@`

1. Connect your own computer to the college eduVPN.
2. Confirm `nc -vz @@SERVER_IP@@ @@HTTPS_PORT@@` and
   `nc -vz @@SERVER_IP@@ @@SSH_PORT@@` succeed.
3. Open `https://@@SERVER_IP@@:@@HTTPS_PORT@@/` and sign in with your own
   college LDAP account.
4. To see all changes you are permitted to read in `@@PROJECT_NAME@@`, open
   `https://@@SERVER_IP@@:@@HTTPS_PORT@@/q/project:@@PROJECT_NAME@@`.
5. For self-signed internal testing, import the administrator-provided
   certificate and verify its fingerprint. Never disable certificate validation.
6. Generate your key locally with
   `ssh-keygen -t ed25519 -C "your.college.email@example"`.
7. In Gerrit, open **Settings → SSH Keys** and upload only `id_ed25519.pub`.
   Never upload or share `id_ed25519`.
8. Test with
   `ssh -p @@SSH_PORT@@ your_gerrit_username@@@SERVER_IP@@`.
9. Clone with
   `git clone ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@`.
10. In the clone, install the hook:

   ```bash
   scp -P @@SSH_PORT@@ your_gerrit_username@@@SERVER_IP@@:hooks/commit-msg .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg
   ```

11. Create a branch, commit, verify the `Change-Id`, and run
    `git push origin HEAD:refs/for/main`.
12. Open another user's change, click **Reply**, add comments and select a
    Code-Review vote.
13. Revise your change with `git commit --amend`, keeping its `Change-Id`, then
    push to `refs/for/main` again.
14. Submit only after the configured review requirements pass and only if your
    group has Submit permission.
15. Report a lost device or compromised key immediately so the administrator
    can revoke that specific public key.

Your Gerrit username can differ from your college Linux username. Never reuse
another person's account, browser session, or SSH key.
