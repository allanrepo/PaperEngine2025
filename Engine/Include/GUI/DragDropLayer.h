#pragma once

#pragma region // Documentation
/********************************************************************************************************************************************** 
Drag & Drop System — Design & Architecture

1. Purpose
---------------------------------------------------------------------------------------------------------
The drag-and-drop system allows a widget to be dragged from its current parent 
and dropped onto another widget that accepts dropped widgets.

The system is intended for UI scenarios such as:
    - inventory systems
    - skill bars
    - customizable menus
    - item placement
    - rearranging UI elements
    - other widget-to-widget drag-and-drop interactions

The implementation uses a dedicated DragDropLayer as a temporary parent for the widget being dragged.

The fundamental operation is:

    Original Parent
          │
          │ Begin Drag
          ▼
    DragDropLayer
          │
          │ End Drag
          ├───────────────┐
          │               │
     Valid Drop        Invalid Drop
          │               │
          ▼               ▼
    New Parent       Original Parent


2. High-Level Architecture
---------------------------------------------------------------------------------------------------------
The drag-and-drop system consists of three main participants:

                         UISystem
                            │
                ┌───────────┴───────────┐
                │                       │
                ▼                       ▼
           Draggable              DragDropLayer
                │                       │
                │                       │
                └────── requests ──────►│
                                        │
                                        ▼
                                   Dragged Widget

Their responsibilities are intentionally separated:

Draggable
    - A widget that can initiate a drag operation.
    - It does not know about DragDropLayer.
    - It simply asks the UISystem to begin or end a drag.

UISystem
    - Coordinates input and determines the drag/drop target.
    - It performs hit testing and passes the resulting target to DragDropLayer.

DragDropLayer
    - Owns the widget currently being dragged and manages its temporary reparenting.
    - It remembers the widget's original parent and position so the widget can either be 
      dropped into a new parent or restored to its original location.


3. DragDropLayer Is Not a LayerStack Layer
---------------------------------------------------------------------------------------------------------
DragDropLayer is a Widget, but it is deliberately not part of the normal LayerStack.

It is owned directly by UISystem:

    class UISystem
    {
    private:
        LayerManager m_layerManager;
        TooltipManager m_tooltipManager;
        DragDropLayer m_dragDropLayer;
    };

This distinction is important.
The normal Layer abstraction represents an independent UI tree participating in the application's layer stack.
DragDropLayer instead represents a temporary system-owned widget container.
Its purpose is specifically to provide a temporary parent for the widget being dragged.

Conceptually:

    UISystem
    │
    ├── LayerManager
    │     └── Layer stack
    │
    ├── TooltipManager
    │     └── Tooltip
    │
    └── DragDropLayer
          └── currently dragged widget

The drag layer therefore does not need:
    - z-order stack semantics
    - modal behavior
    - layer ownership
    - layer routing
    - independent interaction state

It is simply a root-like widget container used during an active drag.


4. Draggable
---------------------------------------------------------------------------------------------------------
Draggable derives from Widget.
Its purpose is to provide a convenient widget type whose mouse events initiate drag-and-drop behavior.

On mouse down:
    system->BeginDrag(this);

On mouse up:
    system->EndDrag(this, position);

The widget therefore knows only about the UISystem interface.

It does not know:
    - how the drag is stored
    - where the dragged widget is temporarily placed
    - how drop targets are found
    - how the original parent is restored
    - how the widget is rendered while dragging

This keeps drag/drop management outside the draggable widget itself.


5. Drag Initiation
---------------------------------------------------------------------------------------------------------
Currently, a drag begins when the draggable receives MouseDown.

The sequence is:
    MouseDown
        │
        ▼
    Draggable::OnMouseDown()
        │
        ▼
    UISystem::BeginDrag()
        │
        ▼
    DragDropLayer::Begin()

The current implementation intentionally begins the drag immediately rather than waiting for the mouse to move a minimum distance.
This is acceptable for the current implementation.
The distinction between a simple click and an actual drag may be introduced later if required.


6. Temporary Reparenting
---------------------------------------------------------------------------------------------------------
The central design decision is that the dragged widget itself is moved into DragDropLayer.
The system does not create a visual proxy for the dragged widget.

Before dragging:

    Original Parent
        │
        └── Draggable

During dragging:

    Original Parent

    DragDropLayer
        │
        └── Draggable

After a successful drop:

    New Parent
        │
        └── Draggable

After an unsuccessful drop:

    Original Parent
        │
        └── Draggable

This means the actual widget continues to exist throughout the operation.
Its identity, state, children, and widget-specific behavior remain intact.
Only its parent changes.


7. DragDropContext
---------------------------------------------------------------------------------------------------------
Before reparenting the widget, DragDropLayer stores the information required to restore it:

    struct DragDropContext
    {
        Widget* originalParent = nullptr;
        PositionF originalPosition;
    };

Two pieces of information are currently required.

    Original parent
    Widget* originalParent;

This is used if the drop target is invalid.

    Original position
    PositionF originalPosition;

This is the position relative to the original parent.

If the drop fails, the widget can therefore be restored to its exact original location.


8. Preserving Screen Position During Reparenting
---------------------------------------------------------------------------------------------------------
Reparenting a widget changes the coordinate system in which its position is interpreted.
Before reparenting, the draggable's position is relative to its original parent.
Therefore DragDropLayer::Begin() first obtains its absolute position:

    PositionF pos = draggable->GetAbsolutePosition();

The widget is then moved to the drag layer:

    draggable->GetParent()->MoveChildTo(draggable, this);

Finally, the absolute position is converted into a position relative to the drag layer:

    pos = pos - GetAbsolutePosition();
    draggable->SetPosition(pos);

The invariant is:

    Beginning a drag must not visually move the draggable.
    The widget changes parent, but its screen position remains unchanged.
    Although DragDropLayer is currently root-like at {0,0}, the coordinate conversion is deliberately 
    explicit because the correct operation is an absolute-to-relative conversion.


9. Active Drag Invariant
---------------------------------------------------------------------------------------------------------
The current system intentionally supports only one active drag operation at a time.
DragDropLayer::Begin() enforces this by checking both its context storage and its children.

Conceptually:

    DragDropLayer

Idle:
    0 dragged widgets

Dragging:
    1 dragged widget

Starting another drag while one is already active is considered an invalid state and results in an exception.
This is a deliberate invariant of the current implementation.


10. Drag Context Storage
---------------------------------------------------------------------------------------------------------
The current implementation uses:

    Dictionary<Widget*, DragDropContext> m_draggables;

The dictionary was chosen with a future requirement in mind: supporting multiple simultaneous drag operations.
However, the current implementation deliberately restricts the system to one active drag.

Therefore the current architecture is:

Storage:
    potentially supports multiple draggables

Current invariant:
    exactly zero or one draggable

The dictionary can therefore remain as preparation for future multi-drag support.


11. Drop Target Detection
---------------------------------------------------------------------------------------------------------
DragDropLayer does not perform hit testing itself.
When the mouse is released, UISystem determines what widget is underneath the mouse.

The sequence is:

    MouseUp
       │
       ▼
    Draggable::OnMouseUp()
       │
       ▼
    UISystem::EndDrag()
       │
       ▼
    LayerManager hit testing
       │
       ▼
    target Widget*
       │
       ▼
    DragDropLayer::End()

This is an intentional responsibility boundary.

UISystem owns knowledge of:
    - mouse position
    - layers
    - modal blocking
    - widget hit testing
    - z-order resolution

DragDropLayer does not duplicate any of that logic.


12. Droppable Widgets
---------------------------------------------------------------------------------------------------------
A widget can indicate that it accepts dropped widgets through:

    IsDroppable()

During DragDropLayer::End(), the requested target is checked:

No target
    → restore original parent

Target exists but is not droppable
    → restore original parent

Target exists and is droppable
    → reparent to target

The current drop contract is intentionally simple:

    IsDroppable()
        ↓
    Can this widget receive a dropped widget?

If the target is accepted, the dragged widget becomes its child.


13. Successful Drop
---------------------------------------------------------------------------------------------------------
For a valid target:

    Dragged Widget
          │
          ▼
    Droppable Target

The widget's absolute position is captured before changing its parent:

    draggable->GetAbsolutePosition()

Then the widget is moved into the target:

    MoveChildTo(draggable, parent);

The absolute position is converted into a position relative to the new parent:

    absolutePosition - parent->GetAbsolutePosition()

This preserves the widget's screen position when it is dropped.

The invariant is:
    Changing the widget's parent during a drop must not unexpectedly change its screen position.


14. Failed Drop
---------------------------------------------------------------------------------------------------------
If the target is invalid or not droppable, the widget returns to its original parent:

Invalid drop
     │
     ▼
Original Parent
     │
     ▼
Original Position

The original context is used:

    context.originalParent
    context.originalPosition

This provides a simple cancel/revert behavior.


15. Drop Semantics
---------------------------------------------------------------------------------------------------------
The current implementation treats a droppable target as the new parent.

Therefore:
    IsDroppable() == true

currently means:
    This widget may become the parent of the dragged widget.

This is intentionally simple.

The system does not currently distinguish between:

    Can I accept this draggable?

    and:

    What should I do when this draggable is dropped?

That distinction may be introduced later if actual application requirements demand it.


16. Ownership During Drag
---------------------------------------------------------------------------------------------------------
During a drag, the dragged widget is owned by DragDropLayer through the normal widget child ownership mechanism.

Before the drag:

    Original Parent
        └── unique_ptr<Draggable>

During the drag:

    DragDropLayer
        └── unique_ptr<Draggable>

After the drop:

    New Parent
        └── unique_ptr<Draggable>

This means there is no separate ownership system for dragged widgets.
The widget always belongs to exactly one widget tree.
The drag operation simply changes which widget owns it.


17. Lifetime Invariant
---------------------------------------------------------------------------------------------------------

The drag context contains a non-owning pointer:

    Widget* originalParent;

The original parent must therefore remain alive for the duration of the drag.
This is an architectural invariant rather than something that DragDropLayer should recover from.

If the original parent can be destroyed while its child is being dragged, 
the underlying widget lifecycle/ownership behavior is incorrect and should be fixed at that level.

The drag system should not silently attempt to recover from an invalid original parent.

The intended invariant is:
    The original parent remains valid for the lifetime of the active drag operation.

The same general principle applies to the dragged widget itself: 
    a draggable should not disappear independently while it is being tracked by DragDropLayer.


18. Relationship to Widget DragHandler
---------------------------------------------------------------------------------------------------------
Widget already contains a DragHandler responsible for widget movement.

This is related to, but distinct from, DragDropLayer.

Widget::DragHandler is concerned with moving a widget.

DragDropLayer is concerned with temporarily changing the parent of a widget 
so that it can be dragged and dropped between widgets.

Therefore the two mechanisms should not automatically be merged.

Conceptually:

    DragHandler
        = movement behavior

    DragDropLayer
        = temporary drag/drop ownership

A future implementation may use the two together, but they solve different problems.


19. System Responsibility
---------------------------------------------------------------------------------------------------------
UISystem acts as the coordinator.

It exposes:

    BeginDrag(Widget*)
    EndDrag(Widget*, PositionF)

and delegates the actual drag state management to DragDropLayer.

The system determines the target using the normal UI hit-testing infrastructure.
This keeps drag/drop consistent with the rest of the UI input architecture.

The system already understands:
    - layer ordering
    - modal blocking
    - widget visibility
    - widget enabled state
    - z-order
    - mouse capture

The drag/drop implementation therefore does not need another hit-testing mechanism.


20. Rendering
---------------------------------------------------------------------------------------------------------
Because the dragged widget becomes a child of DragDropLayer, it is rendered separately from the normal layer stack.

UISystem::Draw() renders:

1. Layer stack
2. Tooltip
3. DragDropLayer

Therefore the dragged widget is visually presented as a top-level drag presentation.
This is one of the reasons the dedicated drag layer exists.
The widget can remain visually above the normal UI while being dragged without becoming a member of the normal LayerStack.


21. Design Invariants
---------------------------------------------------------------------------------------------------------
The current implementation relies on the following invariants:
    There is at most one active drag operation.
    An active draggable is a child of DragDropLayer.
    A draggable must have a parent when dragging begins.
    The original parent remains alive for the duration of the drag.
    The dragged widget itself remains alive for the duration of the drag.
    Beginning a drag does not change the widget's screen position.
    Dropping a widget does not unexpectedly change its screen position.
    An invalid drop returns the widget to its original parent and position.
    UISystem performs hit testing; DragDropLayer does not.
    A droppable widget may become the parent of the dragged widget.
    DragDropLayer is not part of LayerStack.
    The dragged widget remains the actual widget; no proxy representation is created.
    The drag layer is system-owned and root-like.
    Dragging does not create a second ownership model; it temporarily changes widget-tree ownership.


22. Current Architecture
---------------------------------------------------------------------------------------------------------

The complete current design can be summarized as:

                         UISystem
                            │
              ┌─────────────┴─────────────┐
              │                           │
       input / hit testing           DragDropLayer
              │                           │
              │                           │
              ▼                           ▼
          target Widget             dragged Widget
              │                           │
              │                           │
              └─────────────┬─────────────┘
                            │
                            ▼
                     Drop decision
                            │
                  ┌─────────┴─────────┐
                  │                   │
             droppable             invalid
                  │                   │
                  ▼                   ▼
             new parent         original parent

The central mechanism is temporary reparenting:

Normal:
    Parent → Widget

Dragging:
    DragDropLayer → Widget

Successful Drop:
    NewParent → Widget

Failed Drop:
    OriginalParent → Widget

This provides drag-and-drop behavior without introducing a separate proxy object or separate ownership mechanism.


23. Future Work / TODO
---------------------------------------------------------------------------------------------------------

The following items are intentionally deferred.

TODO 1 — Multiple simultaneous drags

    The current implementation enforces one active drag.

    The Dictionary<Widget*, DragDropContext> was chosen because the intended long-term design may support multiple simultaneous drags.

    When implementing this:

    remove the single-drag invariant
    determine how multiple dragged widgets are represented visually
    determine whether each draggable requires its own drag context
    determine how mouse input identifies which drag operation is being updated
    determine whether multiple input devices/pointers need to be supported

    Do not implement this until there is an actual requirement for simultaneous dragging.

TODO 2 — Original parent lifetime

    Currently the drag context stores:

        Widget* originalParent;

    The intended invariant is that the original parent cannot be destroyed while the drag is active.
    Verify that the widget lifecycle guarantees this.
    If the invariant can be violated, fix the underlying ownership/lifecycle behavior rather than adding silent recovery logic to DragDropLayer.

TODO 3 — Drag initiation threshold

    Currently:

        MouseDown
            ↓
        BeginDrag

        Future behavior could distinguish a click from an actual drag:

        MouseDown
            ↓
        Potential drag
            ↓
        MouseMove
            ↓
        movement exceeds threshold
            ↓
        BeginDrag

    This would prevent a simple click from technically entering drag state.
    Whether this is necessary depends on the desired interaction model.

TODO 4 — More expressive drop contract

    Currently:

        IsDroppable()

    determines whether a widget can receive the draggable.

    Future requirements may require more information, for example:

        Can this target accept this particular draggable?

        or:

        What should happen when this draggable is dropped?

    Potential future mechanisms could include target-specific acceptance and drop notifications.
    Do not introduce these until actual drag/drop use cases require them.

TODO 5 — Drop notification

    The current implementation contains:

        // TODO: do we need to notify anyone when drop happened?

    This should remain open until real application scenarios establish what needs to happen after a successful drop.

    Potential consumers might include:
        inventory systems
        skill bars
        item swapping
        menu rearrangement
        application/game state updates

    The drag/drop infrastructure should only provide the notification mechanism required by those use cases.


24. Final Design Principle
---------------------------------------------------------------------------------------------------------
The drag-and-drop system is based on a simple principle:

A drag operation temporarily changes the parent of the actual widget being dragged.

Everything else follows from that:

Draggable
    asks UISystem to begin/end

UISystem
    determines the drop target

DragDropLayer
    temporarily owns the dragged widget

DragDropContext
    remembers how to restore it

Droppable Widget
    can receive the dragged widget

This keeps the drag/drop mechanism small while allowing it to evolve later as actual UI requirements become more sophisticated.


**********************************************************************************************************************************************/

#pragma endregion

#pragma region // include files
#include <GUI/Widget.h>
#include <Containers/Dictionary.h>
#pragma endregion

namespace engine
{
    namespace gui
    {
#pragma region // forward declarations
        template<typename K, typename T>
        using Dictionary = engine::container::Dictionary<K, T>;

#pragma endregion

#pragma region // DragDropLayer
        // this class is a widget layer in UI system. there should be only one of this in a UI system
        // it's purpose is to store current widgets that are in drag/drop state.
        // at the beginning of drag state, it adopts the dragged widget as its child and manages its movement
        // at the end of drag state (drop), it release the dragged widget into appropriate droppable target widget
        // if there is no appropriate droppable target widget, it returns it to original parent
        class DragDropLayer : public Widget
        {
            // context to remember information about a widget being dragged
            struct DragDropContext
            {
                Widget* originalParent = nullptr;
                PositionF originalPosition;
            };

        private:
            friend UISystem;

            UISystem* m_system;
            Dictionary<Widget*, DragDropContext> m_draggables;

        protected:
            UISystem* GetSystem() const override final;

        protected:
            DragDropLayer(UISystem* system);

            // this method is called when a drag/drop state on a given widget is about to begin
            void Begin(Widget* draggable);

            void End(Widget* draggable, Widget* newParent);
        };
#pragma endregion

#pragma region // Draggable
        // a widget that can be dragged from one droppable widget into another
        // it's used for inventory systems, skill bars, customizable menus, etc...
        class Draggable : public Widget
        {
        private:
        protected:
            // this widget is draggable via mouse move so we handle start of dragging through mouse down
            virtual void OnMouseDown(const PositionF& position);

            // this widget drops on mouse up
            virtual void OnMouseUp(const PositionF& position);

        public:
            Draggable();

            void Draw(const UIDrawContext& context) const override;
        };
#pragma endregion

    }
}