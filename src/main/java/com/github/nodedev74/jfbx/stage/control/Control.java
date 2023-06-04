package com.github.nodedev74.jfbx.stage.control;

public abstract class Control {

    private boolean active = true;

    protected void delete() {
        this.active = false;
    }

    public boolean isActive() {
        return active;
    }

    public abstract void lifecycle();
}
