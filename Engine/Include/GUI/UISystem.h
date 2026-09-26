#pragma once
#pragma region // Design Document
/****************************************************************************************************************
UISystem — Design & Architecture

-----------------------------------------------------------------------------------------------------------------
1. Purpose
-----------------------------------------------------------------------------------------------------------------
The UISystem is the central coordinator of the UI framework.
It coordinates functionality that requires knowledge of the UI as a whole rather than the internal 
behavior of an individual Widget.

The system is responsible for:
    - Layer management
    - Mouse input routing
    - Keyboard input routing
    - Mouse capture
    - Mouse-over tracking
    - Keyboard focus
    - Tooltip coordination
    - Drag-and-drop coordination
    - UI-wide resources
    - UI rendering coordination

The UISystem does not own individual Widgets.
Instead, it coordinates the Widget and Layer hierarchy while delegating specialized responsibilities 
to dedicated components.

The fundamental relationship is:

                         UISystem
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
          ▼                 ▼                 ▼
     LayerManager     TooltipManager    DragDropLayer
          │
          ▼
       Layer
          │
          ▼
     Widget tree

-----------------------------------------------------------------------------------------------------------------
2. High-Level Architecture
-----------------------------------------------------------------------------------------------------------------
The UISystem consists of several cooperating components:

    class UISystem
    {
    private:
        LayerManager m_layerManager;
        TooltipManager m_tooltipManager;
        DragDropLayer m_dragDropLayer;
        InteractionStateTracker m_interactionStateTracker;
        UIResources m_resources;
    };

Each component has a distinct responsibility.

    LayerManager
        - Owns and manages the Layer stack.
        - Handles Layer creation, removal, expansion, collapse and routing.
        - Processes deferred Layer commands.

    TooltipManager
        - Manages tooltip behavior.
        - Determines the tooltip presentation independently from Layer management.

    DragDropLayer
        - Temporarily owns a Widget while it is being dragged.
        - Provides the temporary Widget parent required by drag-and-drop.

    InteractionStateTracker
        - Tracks focus, mouse capture and mouse-over.
        - Maintains non-owning references to Widgets participating in these states.
        - Safely clears those references when Widgets unregister.

    UIResources
        - Stores UI-wide resources such as fonts.

The UISystem coordinates these components but does not absorb their implementation details.

-----------------------------------------------------------------------------------------------------------------
3. Ownership
-----------------------------------------------------------------------------------------------------------------
The UISystem owns its internal UI-management components directly:

    Application
        │
        ▼
    UISystem
        │
        ├── LayerManager
        │       └── Layer stack
        │               └── Widget trees
        │
        ├── TooltipManager
        │
        ├── DragDropLayer
        │       └── dragged Widget
        │
        ├── InteractionStateTracker
        │       └── non-owning Widget references
        │
        └── UIResources


Individual Widgets are owned by their Widget tree.

For example:

    Layer
      │
      └── unique_ptr<Widget>
              │
              └── unique_ptr<Widget>
                      │
                      └── ...

UISystem therefore does not maintain a second ownership model for Widgets.

The important distinction is:

    Ownership
        determines lifetime.

    UISystem references
        coordinate interaction with existing objects.


-----------------------------------------------------------------------------------------------------------------
4. Root Layer
-----------------------------------------------------------------------------------------------------------------
The UISystem always contains a root Layer.
The root Layer is created internally during UISystem construction.
Construction is intentionally performed through the same LayerManager command mechanism:

    Layer::BuildDescription root
    {
        PositionF{0, 0},
        SizeF{0, 0},
        nullptr,
        Layer::Modal,
        false
    };

    m_layerManager.QueueAdd(root);
    m_layerManager.ProcessCommandRequests();

The root Layer therefore becomes the bottom-most Layer in the Layer stack.

Conceptually:

    Layer stack

        Top
         │
         ├── application Layers
         │
         ├── Popup / Menu / Modal Layers
         │
         └── Root Layer
                 │
                 └── application Widgets


The root Layer acts as the permanent base of the UI system.


-----------------------------------------------------------------------------------------------------------------
5. Layer Management
-----------------------------------------------------------------------------------------------------------------
UISystem provides the application-facing interface for Layer operations while
LayerManager contains the actual Layer-stack implementation.

Public operations include:
    AddLayer()
    Collapse()

These represent legitimate application-level operations.
Other operations are restricted to internal or specialized UI functionality:

    RemoveLayer()
    ToggleLayer()
    IsLayerExpanded()

-----------------------------------------------------------------------------------------------------------------
6. Deferred Layer Changes
-----------------------------------------------------------------------------------------------------------------
Layer-stack modifications are deferred.

This is necessary because Layer operations can be requested while the UI is
currently processing input.

For example:

    MouseDown()
        │
        ├── determine clicked Layer
        │
        ├── QueueCollapseAbove()
        │
        ├── Widget::MouseDown()
        │       │
        │       └── MenuButton requests ToggleLayer()
        │
        └── input processing completes
                    │
                    ▼
             Process commands


The Layer hierarchy is therefore not immediately modified in the middle of
input traversal.

UISystem provides two synchronization points:

    Begin()
        │
        ▼
    FlushCommands()

and:

    End()
        │
        ▼
    ProcessCommandRequests()


Begin() applies commands that were already pending before the current UI
processing scope.

End() processes commands generated during the current processing scope.

-----------------------------------------------------------------------------------------------------------------
7. Mouse Input Routing
-----------------------------------------------------------------------------------------------------------------
UISystem is the external entry point for mouse input.

The general routing process is:

    Application
        │
        ▼
    UISystem
        │
        ▼
    LayerManager::FindRouteFromTopAt()
        │
        ├── modal blocking
        │
        └── target Layer
                │
                ▼
        Widget hit testing
                │
                ▼
        target Widget


Layer routing is performed before Widget hit testing.

This allows modal behavior and Layer ordering to be resolved independently from
the Widget hierarchy.

-----------------------------------------------------------------------------------------------------------------
8. MouseDown
-----------------------------------------------------------------------------------------------------------------
MouseDown() first flushes pending Layer commands.
It then determines which Layer is under the mouse.

    MouseDown(p)
        │
        ▼
    FlushCommands()
        │
        ▼
    FindRouteFromTopAt(p)
        │
        ├── blocked by Modal
        │
        └── Layer found
                │
                ▼
        FindAndResolveZOrderAt()
                │
                ▼
              Widget

If the route is blocked by a Modal Layer, UISystem does not send the mouse
event to an underlying Layer.

Instead, Layers above the blocking Layer may be collapsed.

If a valid Widget is found:

    1. Queue CollapseAbove()
    2. Call Widget::MouseDown()
    3. Set mouse capture
    4. Set focus
    5. Hide tooltip

The CollapseAbove operation is deliberately queued before Widget::MouseDown()
is executed.

This allows a LayerTrigger such as a MenuButton to request its own Layer
operation during MouseDown without the Layer being immediately destroyed by
the collapse operation.

-----------------------------------------------------------------------------------------------------------------
9. Mouse Capture
-----------------------------------------------------------------------------------------------------------------
Mouse capture ensures that a Widget which has started an interaction continues
to receive mouse events even when the cursor leaves its normal bounds.

After MouseDown():

    clicked Widget
        │
        ▼
    SetCapture(widget)


While capture is active:

    MouseMove()
        │
        ▼
    captured Widget::MouseMove()


    MouseUp()
        │
        ▼
    captured Widget::MouseUp()
        │
        ▼
    SetCapture(nullptr)

Mouse capture therefore takes priority over normal hit testing.

-----------------------------------------------------------------------------------------------------------------
10. MouseMove
-----------------------------------------------------------------------------------------------------------------
MouseMove() has two distinct paths.

If a Widget has mouse capture:

    MouseMove()
        │
        ▼
    captured Widget
        │
        ▼
    MouseMove()

Tooltip display is suppressed while capture is active.

If there is no capture:

    MouseMove()
        │
        ▼
    FindRouteFromTopAt()
        │
        ├── blocked by Modal
        │
        └── Layer
                │
                ▼
        FindTopWidgetAt()
                │
                ▼
           mouse-over Widget


The mouse-over Widget is then compared with the previously tracked Widget.

If it changed:

    old Widget
        │
        ▼
    MouseLeave()

    new Widget
        │
        ▼
    MouseEnter()

The new Widget then receives MouseMove().

Finally:

    TooltipManager::Show(mouseOver)

Disabled Widgets are included in the mouse-over search so that a disabled
Widget may still participate in tooltip display.

-----------------------------------------------------------------------------------------------------------------
11. MouseUp
-----------------------------------------------------------------------------------------------------------------
MouseUp() is routed through the current mouse capture.

If there is no captured Widget, the event is ignored.

Otherwise:
    captured Widget
        │
        ▼
    Widget::MouseUp()
        │
        ▼
    SetCapture(nullptr)

After mouse capture is released, normal mouse-over processing resumes on the
next MouseMove().

-----------------------------------------------------------------------------------------------------------------
12. Keyboard Input
-----------------------------------------------------------------------------------------------------------------
Keyboard input is routed through the current focus Widget.

    KeyDown(key)
        │
        ▼
    Focused Widget
        │
        ▼
    Widget::KeyDown()

Similarly:

    KeyUp(key)
        │
        ▼
    Focused Widget
        │
        ▼
    Widget::KeyUp()

UISystem therefore does not perform keyboard hit testing.

Keyboard input follows focus.

-----------------------------------------------------------------------------------------------------------------
13. InteractionStateTracker
-----------------------------------------------------------------------------------------------------------------
InteractionStateTracker is a private component of UISystem.

It stores:

    Widget* m_mouseCapture;
    Widget* m_mouseOver;
    Widget* m_focus;

These pointers are non-owning.

A Widget may participate in more than one state simultaneously.

For example:

    Widget A
        │
        ├── focus
        └── mouse capture

To manage this, InteractionStateTracker maintains:

    Dictionary<Widget*, int> m_trackedWidgets;

The integer represents how many interaction states currently reference the widget.

For example:

    Widget A
        focus       = yes
        capture     = yes
        mouse-over  = no

    tracked count = 2

The Widget is subscribed to UnregisteredToSystem only once regardless of how
many interaction states reference it.

-----------------------------------------------------------------------------------------------------------------
14. Interaction State Lifetime
-----------------------------------------------------------------------------------------------------------------
The InteractionStateTracker must not retain a stale Widget pointer after the widget leaves the UI system.

When a Widget becomes tracked:

    Track(widget)
        │
        ▼
    subscribe to UnregisteredToSystem

When it is no longer referenced by any interaction state:

    Untrack(widget)
        │
        ▼
    unsubscribe

If the Widget unregisters while still being tracked:

    Widget
        │
        ▼
    UnregisteredToSystem
        │
        ▼
    InteractionStateTracker::OnWidgetUnregistered()
        │
        ├── clear capture
        ├── clear focus
        └── clear mouse-over

This allows interaction state to follow Widget lifetime without making
InteractionStateTracker an owner of the Widget.

-----------------------------------------------------------------------------------------------------------------
15. Focus
-----------------------------------------------------------------------------------------------------------------
Focus changes are managed centrally.

When focus changes:

    Current Focus
        │
        ▼
    OnLostFocus()
        │
        ▼
    remove tracking
        │
        ▼
    New Focus
        │
        ▼
    OnGotFocus()
        │
        ▼
    add tracking

A Widget must be focusable before it can become the new focus Widget.

Setting focus to nullptr is always allowed and simply clears the current
focus.

-----------------------------------------------------------------------------------------------------------------
16. Mouse-Over State
-----------------------------------------------------------------------------------------------------------------
UISystem determines mouse-over state through Layer and Widget hit testing.
The Widget itself does not globally determine which Widget is under the
mouse.

The sequence is:

    MouseMove()
        │
        ▼
    Layer routing
        │
        ▼
    Widget hit testing
        │
        ▼
    new mouse-over Widget
        │
        ├── old != new
        │       │
        │       ├── old::MouseLeave()
        │       └── new::MouseEnter()
        │
        ▼
    new::MouseMove()

Mouse-over is therefore a UISystem-level interaction state.

-----------------------------------------------------------------------------------------------------------------
17. Modal Blocking
-----------------------------------------------------------------------------------------------------------------
Modal behavior is determined by LayerManager.
UISystem consumes the routing result rather than implementing modal Layer
rules itself.

If the mouse position is outside the active Modal Layer:

    Mouse input
        │
        ▼
    LayerManager
        │
        ▼
    isBlockedByModal == true
        │
        ├── do not hit-test underlying Layers
        ├── clear mouse-over if necessary
        └── hide tooltip


The same principle applies to MouseDown.

A Modal Layer therefore acts as an input boundary for Layers beneath it.

-----------------------------------------------------------------------------------------------------------------
18. Tooltip Coordination
-----------------------------------------------------------------------------------------------------------------
Tooltip behavior is delegated to TooltipManager.
UISystem determines the current mouse-over Widget and tells TooltipManager
when to show or hide the tooltip.

Typical flow:

    MouseMove()
        │
        ▼
    determine mouse-over Widget
        │
        ▼
    TooltipManager::Show(widget)


Tooltips are hidden when:
    - MouseDown occurs
    - mouse capture is active
    - mouse movement is blocked by a Modal Layer


Tooltip rendering is performed separately from normal Layer rendering.

-----------------------------------------------------------------------------------------------------------------
19. Drag-and-Drop Coordination
-----------------------------------------------------------------------------------------------------------------
Drag-and-drop is delegated to DragDropLayer.
UISystem is responsible for determining the drop target.

The flow is:

    Draggable
        │
        ▼
    UISystem::BeginDrag()
        │
        ▼
    DragDropLayer::Begin()


When the drag ends:

    Draggable
        │
        ▼
    UISystem::EndDrag()
        │
        ▼
    LayerManager hit testing
        │
        ▼
    target Widget
        │
        ▼
    DragDropLayer::End()


UISystem already owns the knowledge required for target resolution:
    - Layer ordering
    - Modal blocking
    - Visibility
    - Enabled state
    - Widget hit testing
    - Z-order


DragDropLayer therefore does not duplicate this logic.

-----------------------------------------------------------------------------------------------------------------
20. UI Resources
-----------------------------------------------------------------------------------------------------------------
UISystem owns UI-wide resources through UIResources.

Currently the system provides:

    Default font
    Highlight font
    Title font

Resources can be changed through:

    SetFont(font, type)

If the resource actually changed, UISystem broadcasts:

    Widget::ResourceChange()

across the complete Layer/Widget hierarchy.

The purpose is to allow Widgets to recalculate layout or update other
resource-dependent state without requiring every Widget to directly manage
the global UI resource system.

-----------------------------------------------------------------------------------------------------------------
21. Rendering
-----------------------------------------------------------------------------------------------------------------
UISystem coordinates rendering but does not implement Widget rendering itself.

Before drawing, UISystem places current interaction state into UIDrawContext:

    context.capture
    context.hover
    context.focus

The rendering sequence is:

    Layer stack
        │
        ▼
    UIRenderer::Draw()


    Tooltip
        │
        ▼
    UIRenderer::Draw()


    DragDropLayer
        │
        ▼
    UIRenderer::Draw()

The DragDropLayer is rendered after the normal Layer stack so the dragged
Widget can appear above the normal UI.

-----------------------------------------------------------------------------------------------------------------
22. Widget and UISystem Dependency
-----------------------------------------------------------------------------------------------------------------
The base Widget does not need to know about UISystem.

However, specialized Widgets may depend on specific UISystem capabilities.

For example:

MenuButton
    │
    └── LayerTrigger
            │
            └── UISystem Layer operations

and:

    Draggable
        │
        └── UISystem drag operations

This is intentional.

A dependency on UISystem does not imply that UISystem owns the Widget.

The architectural distinction is:

Widget
    owns its children

UISystem
    coordinates system-wide behavior

Specialized Widget
    may request a specific UISystem service

This also prevents every Widget from receiving unrestricted access to the
entire UISystem API.

-----------------------------------------------------------------------------------------------------------------
23. LayerTrigger
-----------------------------------------------------------------------------------------------------------------
LayerTrigger is a type of widget that provides a specialized capability.

Its purpose is to provide controlled access to Layer operations for controls
whose responsibility inherently includes opening, toggling or removing a layer.

Conceptually:

    class LayerTrigger: public Widget
    {
    protected:
        void ToggleLayer();
        void RemoveLayer();

        Layer::BuildDescription m_buildDesc;

        friend class UISystem;
    };

A control may then use the capability through protected implementation inheritance.

For example:

    class MenuButton
        : protected LayerTrigger
    {
    };

This avoids putting Layer-specific functionality into the base Widget class.


-----------------------------------------------------------------------------------------------------------------
24. Frame / Processing Lifecycle
-----------------------------------------------------------------------------------------------------------------
UISystem provides Begin() and End() as synchronization boundaries.

Typical usage is:

    ui.Begin();

    // application / input / UI processing

    ui.End();


Begin():

FlushCommands()

This applies Layer changes that were queued previously.

End():

ProcessCommandRequests()

This processes Layer changes requested during the current processing scope.

The purpose is to establish a predictable point at which deferred Layer
changes become effective.

-----------------------------------------------------------------------------------------------------------------
25. Design Rationale
-----------------------------------------------------------------------------------------------------------------
The UISystem is intentionally a coordinator rather than a large monolithic implementation of every UI subsystem.

    LayerManager owns Layer mechanics.
    TooltipManager owns tooltip mechanics.
    DragDropLayer owns the temporary drag parent.
    InteractionStateTracker owns interaction-state tracking.
    UISystem connects them.

This provides a clear division:

    "Who owns this behavior?"

rather than allowing every Widget to directly manipulate every UI subsystem.

Another important design decision is that UISystem does not attempt to make widget completely unaware of the system.
The base Widget remains independent.

Specialized controls may depend on UISystem when that dependency is inherent
to what the control does.

For example, a MenuButton that exists specifically to toggle a Layer naturally
requires access to Layer functionality.

Trying to eliminate that dependency completely would move the complexity
elsewhere without improving the actual architecture.

-----------------------------------------------------------------------------------------------------------------
26. Design Invariants
-----------------------------------------------------------------------------------------------------------------
The current implementation relies on the following invariants:

    1. UISystem owns its system-level managers but does not own ordinary
       Widgets.

    2. The root Layer always exists at the bottom of the Layer stack.

    3. Widget ownership remains within the Widget hierarchy.

    4. InteractionStateTracker holds non-owning Widget references.

    5. Tracked Widgets unregister themselves through
       UnregisteredToSystem.

    6. A Widget may simultaneously have focus, mouse capture and mouse-over.

    7. Keyboard input is routed through focus.

    8. Mouse input normally begins with Layer routing followed by Widget
       hit testing.

    9. Mouse capture takes priority over normal mouse hit testing.

    10. Modal Layers prevent input from reaching Layers beneath them.

    11. Layer-stack structural changes are deferred.

    12. Widget MouseDown is allowed to request Layer changes before queued
        Layer commands are processed.

    13. Drag-and-drop target detection uses the normal Layer/Widget hit-testing
        infrastructure.

    14. Tooltip presentation is coordinated by UISystem but managed by
        TooltipManager.

    15. UI-wide resources are owned by UISystem.

    16. Base Widget does not require knowledge of UISystem.

    17. Specialized Widgets may depend on narrow UISystem capabilities.

    18. UISystem coordinates subsystems rather than duplicating their
        implementation.

    19. Current Architecture

        The current UISystem architecture can be summarized as:

                                      Application
                                          │
                                          ▼
                                      UISystem
                                          │
               ┌──────────────────────────┼──────────────────────────┐
               │                          │                          │
               ▼                          ▼                          ▼
            LayerManager              InteractionStateTracker     UIResources
            │                          │
            ▼                          ├── Focus
            Layer Stack                    ├── Mouse Capture
            │                          └── Mouse Over
            ▼
            Widget trees

                                      UISystem
                                          │
                           ┌──────────────┴──────────────┐
                           │                             │
                           ▼                             ▼
                     TooltipManager                DragDropLayer
                           │                             │
                           ▼                             ▼
                        Tooltip                   Dragged Widget

        Input therefore flows through UISystem:

            External Input
                  │
                  ▼
              UISystem
                  │
                  ├── LayerManager
                  │       │
                  │       ▼
                  │    Widget
                  │
                  ├── InteractionStateTracker
                  │
                  ├── TooltipManager
                  │
                  └── DragDropLayer

UISystem is consequently the coordination point of the UI framework, while
the actual behavior and ownership remain distributed among the appropriate
subsystems.


-----------------------------------------------------------------------------------------------------------------
30. Future Work
-----------------------------------------------------------------------------------------------------------------
The UISystem architecture is intentionally considered complete enough to be
used by the engine's demo and test applications.

Further refinement should come primarily from actual usage.

Potential future work includes:
    - richer keyboard navigation
    - more sophisticated focus traversal
    - additional UI resources
    - more complete drag-and-drop behavior
    - additional Layer types
    - higher-level control/facade classes
    - additional input devices
    - further Widget interaction features


These should be introduced when concrete UI requirements expose a need for
them.

The current priority is to exercise Widget and UISystem through actual demo
and test code and allow problems or missing capabilities to reveal themselves
naturally.

-----------------------------------------------------------------------------------------------------------------
31. Final Design Principle
-----------------------------------------------------------------------------------------------------------------
UISystem coordinates the UI; it does not become the UI.

Its job is to connect the major pieces of the framework:
    LayerManager
    TooltipManager
    DragDropLayer
    InteractionStateTracker
    UIResources
    Widget hierarchy


The architecture intentionally separates:
    ownership
    from
    dependency


and:
    coordination from implementation.

The result is a system where global UI behavior has a clear home without
forcing every Widget to understand or depend on the entire UI framework.


****************************************************************************************************************/
#pragma endregion

#pragma region // include files
#include <Graphics/Core/Font.h>
#include <GUI/UIRenderer.h>
#include <GUI/Layer.h>
#include <GUI/Tooltip.h>
#include <GUI/DragDropLayer.h>
#pragma endregion

namespace engine
{
    namespace gui
    {
#pragma region // forward declarations
        using Font = engine::graphics::Font;
#pragma endregion

#pragma region // UIResources
        struct UIResources
        {
            Font defaultFont;
            Font highlightFont;
            Font titleFont;

            enum class FontType
            {
                Default,
                Highlight,
                Title
            };

            UIResources()
                : defaultFont(Font::MakeInvalidFont())
                , highlightFont(Font::MakeInvalidFont())
                , titleFont(Font::MakeInvalidFont())
            {
            }
        };
#pragma endregion

#pragma region // UISystem
        class UISystem
        {
        private:
#pragma region // access
            friend class LayerTrigger;
            friend class Draggable;
#pragma endregion

#pragma region // InteractionStateTracker class definition
            // class that tracks focus, mouse capture and hover
            // it maintains non-owning references to Widgets participating in these states.
            // it also safely clears those references when Widgets unregister.
            class InteractionStateTracker
            {
            private:
                Widget* m_mouseCapture = nullptr;
                Widget* m_mouseOver = nullptr;
                Widget* m_focus = nullptr;
                Dictionary<Widget*, int> m_trackedWidgets;

                // event handler for widgets that are unregistered from their UI tree. if they are any of the tracked states, they are removed from tracked state
                void OnWidgetUnregistered(Widget* widget);
                // internal helper method to untrack a widget. this can be focus, capture, or hover widget
                void Untrack(Widget* widget);

                // internal helper method to track a widget. this can be focus, capture, or hover widget
                void Track(Widget* widget);

            public:
                InteractionStateTracker();
                ~InteractionStateTracker();

                void SetFocus(Widget* widget);
                void SetCapture(Widget* widget);
                void SetMouseOver(Widget* widget);

                Widget* GetMouseCapture() const;
                Widget* GetFocus() const;
                Widget* GetMouseOver() const;
            };
#pragma endregion

#pragma region // internal components
            // abstractions
            LayerManager m_layerManager;
            TooltipManager m_tooltipManager;
            DragDropLayer m_dragDropLayer;

            // widget references
            InteractionStateTracker m_interactionStateTracker;

            // UI resource references
            UIResources m_resources;

            // root layer getter. only UISystem get to access this.
            Widget& Root() const;
#pragma endregion

#pragma region // internal operations
            // internal layer operation that immediately remove a layer owned by specified widget. it will also collapse layers on top of it
            bool RemoveLayer(Widget* owner);

            // internal layer operation that toggles a layer owned by specified widget. it is a deferred action
            void ToggleLayer(Widget* owner, const Layer::BuildDescription& desc);

            // internal layer state query
            bool IsLayerExpanded(const Widget* owner) const;

            // internal drag-drop operation where it handles start of a widget being dragged
            void BeginDrag(Widget* source);

            // the position p here is the mouse position where the widget draggable is dragged into
            // the goal of this method is to identify the top widget that intersects with that position p
            void EndDrag(Widget* draggable, const PositionF& p);
#pragma endregion

        public:
#pragma region // general operations
            // constructor
            UISystem();

            // destructor
            ~UISystem();

            // transform and extent setter for root layer
            void SetSize(const SizeF& size);

            void SetPosition(const PositionF& pos);
#pragma endregion

#pragma region // resource management
            // set font for given resource type
            void SetFont(Font font, UIResources::FontType type);

            // get font from given resource type
            Font GetFont(UIResources::FontType type) const;
#pragma endregion

#pragma region // display operations API
            // this makes root layer visible
            void Show();

            // draw via UIDrawContext
            void Draw(UIDrawContext& context);
#pragma endregion

#pragma region // input operations
            // external mouse down event handler
            void MouseDown(const PositionF& p);

            // external mouse up event handler
            void MouseUp(const PositionF& p);

            // external mouse move event handler
            void MouseMove(const PositionF& p);

            // keyboard keydown handler
            void KeyDown(int key);

            // keyboard keyup handler
            void KeyUp(int key);
#pragma endregion

#pragma region // widget operations API
            void AddWidget(std::unique_ptr<Widget> widget);
            void RemoveWidget(Widget* widget);
#pragma endregion

#pragma region // layer operations API
            // collapse all active layers and leave only root layer up
            // this is a deferred action. layers will only collapsed when it is ready to collapse it
            void Collapse();

            // spawn a layer on top of the layer tree. good for spawning modal message box
            // this is a deferred action. layer will only be added into layer stack when it is ready to add it
            void AddLayer(const Layer::BuildDescription& desc);
#pragma endregion

#pragma region // scope operations API
            // call this in application update loop along with its End() to close the UISystem's scope
            void Begin();
            void End();
#pragma endregion

#pragma region // Interaction state queries
            bool HasFocus(const Widget& widget) const;
            bool HasMouseCapture(const Widget& widget) const;
            bool IsMouseOver(const Widget& widget) const;
#pragma endregion
        };
#pragma endregion
    }
}