# Gerrit user configuration

- Server: `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`
- SSH: `@@SERVER_IP@@:@@SSH_PORT@@`
- Project: `@@PROJECT_NAME@@`

1. Connect your own computer to the college eduVPN.
2. Confirm `nc -vz @@SERVER_IP@@ @@HTTPS_PORT@@` and
   `nc -vz @@SERVER_IP@@ @@SSH_PORT@@` succeed.
3. Open `https://@@SERVER_IP@@:@@HTTPS_PORT@@/` without signing in to verify
   that the university-visible review page loads.
4. To see all public changes in `@@PROJECT_NAME@@`, open
   `https://@@SERVER_IP@@:@@HTTPS_PORT@@/q/project:@@PROJECT_NAME@@`.
5. For self-signed internal testing, import the administrator-provided
   certificate and verify its fingerprint. Never disable certificate validation.
6. Ask administrator `@@ADMIN_USERNAME@@` to create your individual login and
   assign the project permissions you require. Sign in with that account.
7. Generate your key locally with
   `ssh-keygen -t ed25519 -C "your.college.email@example"`.
8. In Gerrit, open **Settings → SSH Keys** and upload only `id_ed25519.pub`.
   Never upload or share `id_ed25519`.
9. Test with
   `ssh -p @@SSH_PORT@@ your_gerrit_username@@@SERVER_IP@@`.
10. Open a change's **Download** menu to copy the standard **Checkout**,
    **Cherry Pick**, **Pull**, or **Format Patch** command. Select either SSH or
    authenticated HTTP. The HTTP password is the individual password assigned
    by the administrator; never put it in a command or Git remote URL. Checkout
    and Cherry Pick show the required `git fetch` followed by the corresponding
    `git checkout` or `git cherry-pick` command.
11. Clone with SSH:

    `git clone ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@`

    Or clone through authenticated HTTPS:

    `git clone https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@`

    Git prompts for the password. TLS certificate verification must remain
    enabled.
12. To download files in the change screen, use **Patch File → Zip** for a
    zipped patch or **Archive → TGZ** for a repository snapshot. Gerrit 3.14
    intentionally excludes repository ZIP archives for browser security, so a
    TGZ archive is the standard safe snapshot option.
13. In the clone, install the hook:

   ```bash
   scp -P @@SSH_PORT@@ your_gerrit_username@@@SERVER_IP@@:hooks/commit-msg .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg
   ```

14. Create a branch, commit, verify the `Change-Id`, and run
    `git push origin HEAD:refs/for/@@PROJECT_BRANCH@@`.
15. Open another user's change, click **Reply**, add comments and select a
    Code-Review vote.
16. Revise your change with `git commit --amend`, keeping its `Change-Id`, then
    push to `refs/for/@@PROJECT_BRANCH@@` again.
17. Submit only after the configured review requirements pass and only if your
    group has Submit permission.
18. Report a lost device or compromised key immediately so the administrator
    can revoke that specific public key.

Your Gerrit username can differ from your college Linux username. Never reuse
another person's account, browser session, or SSH key.
