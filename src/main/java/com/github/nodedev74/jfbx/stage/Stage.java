package com.github.nodedev74.jfbx.stage;

import java.util.ArrayList;

import com.github.nodedev74.jfbx.stage.control.Control;

public class Stage {

    public ArrayList<? super Control> children;

    public Stage() {
        children = new ArrayList<>();
    }

    public ArrayList<? super Control> getChildren() {
        return children;
    }

    public void setChildren(ArrayList<? super Control> children) {
        this.children = children;
    }
}
