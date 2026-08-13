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

    function showActionState(actions, activateAction, change) {
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

    function activationHandler(actions, restApi, activateAction, state, reload) {
        return async function activate() {
            const change = state.currentChange;
            if (!change || change.status !== "NEW" || !change.work_in_progress) {
                return;
            }

            actions.setEnabled(activateAction, false);
            try {
                await restApi.post(`/changes/${encodeURIComponent(change._number)}/ready`, {});
                reload();
            } catch (error) {
                actions.setEnabled(activateAction, true);
                globalScope.console.error("Could not activate the Gerrit change", error);
                globalScope.alert("Could not activate the change. Check your Gerrit permission and try again.");
            }
        };
    }

    function install(plugin, reloadPage) {
        const actions = plugin.changeActions();
        const reload = reloadPage || (() => globalScope.location.reload());
        const state = {activate: undefined, activateAction: undefined, currentChange: undefined};
        const updateActionState = (change) => {
            state.currentChange = change;
            if (!state.activateAction) {
                state.activateAction = actions.add(CHANGE_ACTION, "Activate");
                state.activate = activationHandler(
                    actions, plugin.restApi(), state.activateAction, state, reload,
                );
                if (typeof actions.addPrimaryActionKey === "function") {
                    actions.addPrimaryActionKey(state.activateAction);
                }
                actions.addTapListener(state.activateAction, state.activate);
            }
            showActionState(actions, state.activateAction, change);
        };
        plugin.on("showchange", updateActionState);

        return {state, updateActionState};
    }

    const reviewControls = {canShowSubmit, install, requiredSubmitVotesAreSatisfied};

    if (typeof module !== "undefined" && module.exports) {
        module.exports = reviewControls;
    }

    if (globalScope.Gerrit) {
        globalScope.Gerrit.install((plugin) => install(plugin));
    }
}(typeof window === "undefined" ? globalThis : window));
