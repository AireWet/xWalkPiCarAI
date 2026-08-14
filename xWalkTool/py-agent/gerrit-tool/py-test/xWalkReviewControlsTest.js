"use strict";

const assert = require("node:assert/strict");
const test = require("node:test");

const reviewControls = require("../xWalkReviewControls.js");

function submitRequirements(codeReviewStatus, verifiedStatus) {
    return [
        {name: "Code-Review", status: codeReviewStatus},
        {name: "Verified", status: verifiedStatus},
    ];
}

function createActionApi(calls, listeners) {
    return {
        add: (type, label) => {
            calls.push(["add", type, label]);
            return "xwalk-review-controls~activate";
        },
        addPrimaryActionKey: (key) => calls.push(["primary", key]),
        addTapListener: (key, listener) => {
            calls.push(["tap-listener", key]);
            listeners.tap = listener;
        },
        setActionHidden: (...arguments_) => calls.push(["hidden", ...arguments_]),
        setEnabled: (...arguments_) => calls.push(["enabled", ...arguments_]),
        setIcon: (...arguments_) => calls.push(["icon", ...arguments_]),
        setLabel: (...arguments_) => calls.push(["label", ...arguments_]),
        setTitle: (...arguments_) => calls.push(["title", ...arguments_]),
    };
}

function createPlugin(actionApi, listeners, restCalls) {
    return {
        changeActions: () => actionApi,
        on: (event, listener) => {
            listeners[event] = listener;
        },
        restApi: () => ({post: async (...arguments_) => restCalls.push(arguments_)}),
    };
}

function createHarness() {
    const calls = [];
    const listeners = {};
    const actionApi = createActionApi(calls, listeners);
    const restCalls = [];
    const plugin = createPlugin(actionApi, listeners, restCalls);
    let reloadCount = 0;
    reviewControls.install(plugin, () => {
        reloadCount += 1;
    });

    return {calls, listeners, reloadCount: () => reloadCount, restCalls};
}

test("Submit is shown only for an open submittable change with both required votes", () => {
    assert.equal(reviewControls.canShowSubmit({
        status: "NEW",
        submittable: true,
        submit_requirements: submitRequirements("SATISFIED", "SATISFIED"),
    }), true);
    assert.equal(reviewControls.canShowSubmit({
        status: "NEW",
        submittable: false,
        submit_requirements: submitRequirements("SATISFIED", "UNSATISFIED"),
    }), false);
    assert.equal(reviewControls.canShowSubmit({
        status: "MERGED",
        submittable: false,
        submit_requirements: submitRequirements("SATISFIED", "SATISFIED"),
    }), false);
});

test("Action waits for change view", () => {
    const harness = createHarness();

    assert.equal(harness.calls.some((call) => call[0] === "add"), false);
    harness.listeners.showchange({_number: 41, status: "NEW", work_in_progress: true});
    harness.listeners.showchange({_number: 41, status: "NEW", work_in_progress: false});
    assert.equal(harness.calls.filter((call) => call[0] === "add").length, 1);
    assert.equal(harness.calls.filter((call) => call[0] === "tap-listener").length, 1);
});

test("Activate remains visible and becomes a pressed disabled state after activation", () => {
    const harness = createHarness();
    const key = "xwalk-review-controls~activate";

    harness.listeners.showchange({_number: 42, status: "NEW", work_in_progress: true});
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["label", key, "Activate"])));
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["enabled", key, true])));

    harness.listeners.showchange({_number: 42, status: "NEW", work_in_progress: false});
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["label", key, "Activated"])));
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["icon", key, "check"])));
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["enabled", key, false])));
});

test("Merged changes retain the activated state and hide Submit", () => {
    const harness = createHarness();
    const key = "xwalk-review-controls~activate";

    harness.listeners.showchange({
        _number: 43,
        status: "MERGED",
        submittable: false,
        submit_requirements: submitRequirements("SATISFIED", "SATISFIED"),
    });

    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["label", key, "Activated"])));
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify(["enabled", key, false])));
    assert.ok(harness.calls.some((call) => JSON.stringify(call) === JSON.stringify([
        "hidden", "revision", "submit", true,
    ])));
});

test("Activate posts the ready operation once and reloads the change", async () => {
    const harness = createHarness();
    harness.listeners.showchange({_number: 44, status: "NEW", work_in_progress: true});

    await harness.listeners.tap();

    assert.deepEqual(harness.restCalls, [["/changes/44/ready", {}]]);
    assert.equal(harness.reloadCount(), 1);
});
