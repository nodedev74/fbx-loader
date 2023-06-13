package com.github.nodedev74.jfbx.stage;

import java.util.ArrayList;

import com.github.nodedev74.jfbx.stage.control.Control;

/**
 * A Stage that contains the application's children.
 */
public class Stage {

    private ArrayList<? super Control> children;

    /**
     * Constructs the Stage and prepares the children list.
     */
    public Stage() {
        children = new ArrayList<>();
    }

    /**
     * Retrieves the list of children in the Stage.
     *
     * @return The list of children.
     */
    public ArrayList<? super Control> getChildren() {
        return children;
    }

    /**
     * Adds a child to the Stage.
     *
     * @param children The list of children to set.
     */
    public void addChildren(Control child) {
        children.add(child);
    }
}
