#pragma once

/**
 * Model.h
 *
 * Abstract base class defining the lifecycle every renderable shape must
 * follow (InitModel / Render / Resize). Renderer owns shapes only through
 * this interface, so it never needs to know which concrete shape it holds.
 */

class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0; // called once, before the first frame
    virtual void Render() = 0;    // called once per frame
    virtual void Resize(int w, int h) {} // called whenever the surface/window resizes
};