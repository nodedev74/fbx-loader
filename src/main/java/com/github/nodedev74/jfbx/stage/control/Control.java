package com.github.nodedev74.jfbx.stage.control;

/**
 * Abstract base class for controls in a stage.
 */
public abstract class Control {

    private boolean active = true;

    /**
     * Marks the control as deleted by setting its active state to false.
     * Controls that are no longer active will be removed from the stage.
     */
    protected void delete() {
        this.active = false;
    }

    /**
     * Checks if the control is currently active.
     *
     * @return True if the control is active, false otherwise.
     */
    public boolean isActive() {
        return active;
    }

    /**
     * Performs the lifecycle processing for the control.
     * Subclasses must implement this method to define the control's behavior.
     */
    public abstract void lifecycle();
}