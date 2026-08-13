(function (globalScope) {
    "use strict";

    const CHANGE_ACTION = "change";
    const REVISION_ACTION = "revision";
    const READY_ACTION = "ready";
    const SUBMIT_ACTION = "submit";

    function requiredSubmitVotesAreSatisfied(change) {
        const requirements = change.submit_requirements || [];
        const requirementStatus = new Map(
            requirements.map((requirement) => [requirement.name, requirement.status]),
        );

        return requirementStatus.get("Code-Review") === "SATISFIED"
            && requirementStatus.get("Verified") === "SATISFIED";
    }

    function canShowSubmit(change) {
        return change.status === "NEW"
            && change.submittable === true
            && requiredSubmitVotesAreSatisfied(change);
    }

    function install(plugin, reloadPage) {
        const actions = plugin.changeActions();
        const restApi = plugin.restApi();
        const activateAction = actions.add(CHANGE_ACTION, "Activate");
        const reload = reloadPage || (() => globalScope.location.reload());
        let currentChange;

        if (typeof actions.addPrimaryActionKey === "function") {
            actions.addPrimaryActionKey(activateAction);
        }

        function updateActionState(change) {
            currentChange = change;
            const isOpen = change.status === "NEW";
            const isActivated = !change.work_in_progress;

            actions.setActionHidden(CHANGE_ACTION, READY_ACTION, true);
            actions.setActionHidden(REVISION_ACTION, SUBMIT_ACTION, !canShowSubmit(change));
            actions.setLabel(activateAction, isActivated ? "Activated" : "Activate");
            actions.setTitle(
                activateAction,
                isActivated ? "This change is active" : "Activate this change and start Gerrit CI",
            );
            actions.setIcon(activateAction, isActivated ? "check" : "visibility");
            actions.setEnabled(activateAction, isOpen && !isActivated);
        }

        async function activate() {
            if (!currentChange || currentChange.status !== "NEW" || !currentChange.work_in_progress) {
                return;
            }

            actions.setEnabled(activateAction, false);
            try {
                await restApi.post(`/changes/${encodeURIComponent(currentChange._number)}/ready`, {});
                reload();
            } catch (error) {
                actions.setEnabled(activateAction, true);
                globalScope.console.error("Could not activate the Gerrit change", error);
                globalScope.alert("Could not activate the change. Check your Gerrit permission and try again.");
            }
        }

        actions.addTapListener(activateAction, activate);
        plugin.on("showchange", updateActionState);

        return {activate, updateActionState};
    }

    const reviewControls = {canShowSubmit, install, requiredSubmitVotesAreSatisfied};

    if (typeof module !== "undefined" && module.exports) {
        module.exports = reviewControls;
    }

    if (globalScope.Gerrit) {
        globalScope.Gerrit.install((plugin) => install(plugin));
    }
}(typeof window === "undefined" ? globalThis : window));
