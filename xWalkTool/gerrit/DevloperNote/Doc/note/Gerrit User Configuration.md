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
10. Open a change's **Download** menu. Gerrit supplies the real change ref in
    the form `refs/changes/NN/CHANGE_NUMBER/PATCH_SET`; copy that value into
    `CHANGE_REF` before using the commands below:

    ```bash
    CHANGE_REF='refs/changes/NN/CHANGE_NUMBER/PATCH_SET'
    ```

11. Clone with SSH:

    ```bash
    git clone ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@
    ```

12. Fetch and check out a patch set with SSH:

    ```bash
    git fetch ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git checkout FETCH_HEAD
    ```

13. Fetch and cherry-pick a patch set with SSH:

    ```bash
    git fetch ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git cherry-pick FETCH_HEAD
    ```

14. Clone through authenticated HTTPS:

    ```bash
    git clone https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@
    ```

15. Fetch and check out a patch set through authenticated HTTPS:

    ```bash
    git fetch https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git checkout FETCH_HEAD
    ```

16. Fetch and cherry-pick a patch set through authenticated HTTPS:

    ```bash
    git fetch https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git cherry-pick FETCH_HEAD
    ```

    Git prompts for the password. TLS certificate verification must remain
    enabled. Never put the password in the command, Git remote URL, shell
    history, documentation, or repository.
17. To download files in the change screen, use **Patch File → Zip** for a
    zipped patch or **Archive → TGZ** for a repository snapshot. Gerrit 3.14
    intentionally excludes repository ZIP archives for browser security, so a
    TGZ archive is the standard safe snapshot option.
18. In the clone, install the hook:

   ```bash
   scp -P @@SSH_PORT@@ your_gerrit_username@@@SERVER_IP@@:hooks/commit-msg .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg
   ```

19. Create a branch, commit, verify the `Change-Id`, and run
    `git push origin HEAD:refs/for/@@PROJECT_BRANCH@@`.
20. Open another user's change, click **Reply**, add comments and select a
    Code-Review vote.
21. Revise your change with `git commit --amend`, keeping its `Change-Id`, then
    push to `refs/for/@@PROJECT_BRANCH@@` again.
22. Submit only after the configured review requirements pass and only if your
    group has Submit permission.
23. Report a lost device or compromised key immediately so the administrator
    can revoke that specific public key.

Your Gerrit username can differ from your college Linux username. Never reuse
another person's account, browser session, or SSH key.
