#pragma once

#pragma region // Design Document
/****************************************************************************************************************
Tooltip System — Design & Architecture

1. Purpose
-----------------------------------------------------------------------------------------------------------------
The tooltip system provides contextual information for UI widgets when the mouse hovers over them.

The design separates three responsibilities:
    Widget — provides tooltip content when it has one.
    UISystem — determines which widget is currently hovered and whether a tooltip should be shown.
    TooltipManager — owns and manages the actual tooltip instance and controls how tooltip content is presented.

The tooltip system intentionally does not make every widget responsible for tooltip behavior. 
A widget only exposes the capability to provide tooltip content.


2. High-Level Architecture
-----------------------------------------------------------------------------------------------------------------
                         UISystem
                            │
             determines hovered Widget
                            │
                            ▼
                    TooltipManager
                    ┌───────────────┐
                    │ Tooltip       │
                    │   instance    │
                    └───────┬───────┘
                            │
                     builds content
                            │
                            ▼
                         Widget
                    tooltip capability

The important relationship is:

    Widget
      └── provides tooltip content

    UISystem
      └── decides when/which tooltip is requested

    TooltipManager
      └── owns the tooltip and manages its presentation

Widget does not know about TooltipManager.
UISystem does not construct or manage tooltip contents directly.
TooltipManager does not determine which widget the mouse is hovering over.


3. Tooltip Ownership
-----------------------------------------------------------------------------------------------------------------
There is only one tooltip instance in the UI system.

TooltipManager owns that instance:

    class TooltipManager
    {
    private:
        Tooltip m_tooltip;
        Widget* m_owner;
    };

The tooltip is therefore not created for every widget.
Instead, the same tooltip widget is reused:

    Mouse over Button A
            ↓
    build Button A's tooltip
            ↓
    show Tooltip

    Mouse over Button B
            ↓
    clear Button A's tooltip
            ↓
    build Button B's tooltip
            ↓
    show same Tooltip

This makes the tooltip a transient presentation object rather than persistent state belonging to individual widgets.
The widget owns the information that describes its tooltip, while TooltipManager owns the actual tooltip presentation object.


4. Widget's Responsibility
-----------------------------------------------------------------------------------------------------------------
A widget may optionally provide tooltip content.
This is represented by:

    std::function<void(Widget&, Widget&)> m_tooltipBuilder;

A widget therefore has two possible states:

    Widget
     ├── no tooltip
     │
     └── has tooltip
          └── tooltip builder

The widget does not know:
    when the tooltip should appear
    how long the mouse must hover
    whether the mouse is captured
    whether the UI is currently being dragged
    where the tooltip should be positioned
    how long it should remain visible
    who owns the tooltip instance
    what concrete Tooltip object will display it

Those are system-level concerns.
The widget simply supplies its tooltip content.


5. Tooltip Builder
-----------------------------------------------------------------------------------------------------------------
A tooltip is constructed through the widget's builder:

    std::function<void(Widget&, Widget&)>

The two parameters represent:

    Widget& owner
        The widget requesting the tooltip.

    Widget& tooltip
        The widget that receives the tooltip contents.

The manager invokes it as:

    hover->BuildTooltip(m_tooltip);

which eventually calls:

    m_tooltipBuilder(*this, tooltip);

This allows a widget to build content based on itself.
For example, a widget could create:

    Tooltip
     ├── title
     ├── description
     └── additional information

without the tooltip system needing to know what those children are.

The important abstraction is that the builder receives a generic Widget&.
It does not need to know that the destination happens to be the concrete Tooltip class.

the tooltip position will be relative to its owner's space

6. TooltipManager Responsibility
-----------------------------------------------------------------------------------------------------------------
TooltipManager manages the lifecycle of the single tooltip instance.

Its responsibilities include:
    owning the tooltip
    tracking its current owner
    clearing previous tooltip content
    building new tooltip content
    showing the tooltip
    hiding the tooltip
    exposing the tooltip to the renderer

The manager tracks:

    Widget* m_owner;

This identifies which widget currently supplied the tooltip.

When the same widget remains hovered:

    if (hover != m_owner)

is false, so the tooltip does not need to be rebuilt.

When the hovered widget changes, the manager first clears the old tooltip and then rebuilds it for the new owner.


7. Showing a Tooltip
-----------------------------------------------------------------------------------------------------------------

The public operation is:

    void Show(Widget* hover);

The manager receives the widget currently under the mouse.

If the widget is invalid or has no tooltip:

    if (!hover || !hover->HasTooltip())
    {
        Hide();
        return;
    }

Therefore:

    No hovered widget
            │
            ▼
          Hide

    Hovered widget without tooltip
            │
            ▼
          Hide

    Hovered widget with tooltip
            │
            ▼
    Show / rebuild tooltip

If the hovered widget changes, the old tooltip is removed first:

    Hide();
    hover->BuildTooltip(m_tooltip);
    m_owner = hover;

The same tooltip instance is then shown.


8. Hiding a Tooltip
-----------------------------------------------------------------------------------------------------------------
Hiding a tooltip performs more than simply setting its visibility to false.

    void Hide()
    {
        m_tooltip.RemoveChildren();
        m_tooltip.SetPosition({0, 0});
        m_tooltip.SetSize({0, 0});
        m_tooltip.Hide();
        m_owner = nullptr;
    }

The tooltip is therefore returned to a clean state.
This guarantees that when a new owner builds a tooltip, it does not inherit children or layout state from the previous owner.

The lifecycle is effectively:

    Hide
     ↓
    clear old content
     ↓
    reset geometry
     ↓
    hide
     ↓
    clear owner

    followed by:

    Build new content
     ↓
    set new owner
     ↓
    show


9. UISystem Responsibility
-----------------------------------------------------------------------------------------------------------------

UISystem is responsible for determining when a tooltip should be displayed.

It already tracks:

    Widget* m_mouseOver;
    Widget* m_mouseCapture;

During MouseMove(), if there is no mouse capture, the system determines the widget under the mouse.

It then updates:

    m_mouseOver

and finally calls:

    m_tooltipManager.Show(m_mouseOver);

This establishes an important separation:

    UISystem
        "The mouse is currently over Button A."

    TooltipManager
        "Button A has a tooltip. I will display it using my tooltip instance."

The manager does not perform hit testing.


10. Mouse Capture Suppresses Tooltips
-----------------------------------------------------------------------------------------------------------------

A tooltip should not remain visible while another widget has mouse capture.

UISystem::MouseMove() handles this before performing normal hover processing:

    if (m_mouseCapture)
    {
        m_mouseCapture->MouseMove(p);
        m_tooltipManager.Hide();
        return;
    }

Therefore:

    Mouse captured
          ↓
    widget receives MouseMove
          ↓
    tooltip hidden
          ↓
    normal hover processing skipped

This is intentionally a UISystem policy rather than a Widget responsibility.

A widget does not need to know that tooltips are being suppressed during capture.


11. Mouse Down Also Hides the Tooltip
-----------------------------------------------------------------------------------------------------------------

When the mouse is pressed:

    m_tooltipManager.Hide();

This ensures that a tooltip is not left visible while the user is interacting with the UI.

The policy is therefore:

    Mouse hover
        → tooltip may appear

    Mouse capture
        → tooltip hidden

    Mouse down
        → tooltip hidden


12. Tooltip Rendering
-----------------------------------------------------------------------------------------------------------------

The tooltip is rendered separately from the normal layer stack.

UISystem::Draw() first renders the layers:

    m_layerManager.ForEach([&](Widget* widget)
    {
        UIRenderer::Draw(context, *widget);
    });

Then it renders the tooltip:

    UIRenderer::Draw(context, *m_tooltipManager.Get());

Then it renders the drag/drop layer.

The ordering is therefore:

    Layer stack
        ↓
    Tooltip
        ↓
    Drag/drop presentation

This gives the tooltip its intended presentation priority without requiring it to become another Layer.


13. Why Tooltip Is Not a Layer
-----------------------------------------------------------------------------------------------------------------
A tooltip is deliberately different from a layer.

A layer represents an independent widget tree participating in the UI's global interaction stack:

    Layer 0
    Layer 1
    Layer 2
    ...

A tooltip does not have that role.

There is only one tooltip, and it is owned by TooltipManager.

It is a transient presentation object associated with the currently hovered widget.

Therefore:

    Layer
        = independent UI tree + stack semantics

    Tooltip
        = transient presentation widget + manager ownership

Making the tooltip a layer would introduce unnecessary stack semantics for something that does not need them.


14. Tooltip Ownership vs Tooltip Content
-----------------------------------------------------------------------------------------------------------------

A useful distinction in this design is:

    Widget owns:
        "What should my tooltip say/look like?"

    TooltipManager owns:
        "Which tooltip currently exists?"

    UISystem owns:
        "Should a tooltip currently be shown?"

For example:

    Button A
        └── tooltip builder

    Button B
        └── tooltip builder

                  ↓

            TooltipManager
            └── one Tooltip

                  ↑

              UISystem
            current hover

The tooltip content therefore belongs conceptually to the widget, while the tooltip object belongs to the manager.


15. Access Boundaries
-----------------------------------------------------------------------------------------------------------------

Widget currently exposes the tooltip capability through:

    void SetTooltip(std::function<void(Widget&, Widget&)> builder);

The widget can therefore define its tooltip.

The manager consumes the capability through:

    HasTooltip()
    BuildTooltip()

These operations are system-facing rather than normal application-facing operations.

The intended access relationship is:

Application / derived Widget
        │
        │ SetTooltip(...)
        ▼
      Widget
        ▲
        │ HasTooltip()
        │ BuildTooltip()
        │
 TooltipManager

This is why TooltipManager being a friend of Widget is appropriate if HasTooltip() and BuildTooltip() remain protected.
The manager is an authorized consumer of the widget's tooltip capability.


16. Dependency Direction
-----------------------------------------------------------------------------------------------------------------

The dependency direction is intentionally one-way:

    UISystem
        ↓
    TooltipManager
        ↓
    Widget capability

The widget does not depend on the tooltip manager.
In particular, Widget.h does not need to include the concrete Tooltip definition.

The tooltip builder only deals with:

    Widget&

This keeps the base widget independent of the concrete tooltip presentation.

The concrete relationship is hidden inside the manager:

    TooltipManager
        └── Tooltip
              └── Widget

rather than:

    Widget
        └── TooltipManager
              └── Tooltip

That distinction keeps tooltip infrastructure out of the core widget abstraction.


17. Tooltip State Machine
-----------------------------------------------------------------------------------------------------------------

Although no explicit state machine class is necessary, the effective states are simple:

                    ┌──────────────┐
                    │    Hidden    │
                    └──────┬───────┘
                           │
                   hover widget
                   has tooltip
                           │
                           ▼
                    ┌──────────────┐
                    │   Visible    │
                    │ owner = A    │
                    └──────┬───────┘
                           │
                 hover changes to B
                           │
                           ▼
                    ┌──────────────┐
                    │ Rebuild      │
                    │ for B        │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   Visible    │
                    │ owner = B    │
                    └──────────────┘

    Any state
        │
        ├── mouse capture
        ├── mouse down
        ├── no hover
        └── hovered widget has no tooltip
                │
                ▼
              Hidden

An explicit state machine is unnecessary because the state is small and naturally represented by:

Widget* m_owner;
Tooltip m_tooltip;


18. Design Principles
-----------------------------------------------------------------------------------------------------------------
The tooltip system follows several useful architectural principles.

Single ownership:
    There is one clear owner of the tooltip object:

        TooltipManager → Tooltip

    No widget owns the tooltip.

Separation of policy and mechanism:
    UISystem determines whether tooltip display is appropriate.
    TooltipManager performs the actual tooltip management.

Capability-based Widget design:
    A widget does not implement tooltip behavior.

    It merely provides a capability:

        SetTooltip(...) 

No concrete dependency from Widget
    The base Widget does not need to know about Tooltip.
    Tooltip construction operates through the generic Widget interface.

Reuse instead of recreation:
    The same tooltip instance is rebuilt and reused for different owners.

Transient presentation:
    Tooltip contents are disposable UI state. 
    They are rebuilt when the owner changes and removed when the tooltip is hidden.

19. Intended Responsibility Boundary
-----------------------------------------------------------------------------------------------------------------

The complete responsibility boundary can be summarized as:

    ---------------------------------------------------------------------------------------------
    Component	    Responsibility
    ---------------------------------------------------------------------------------------------
    Widget	        Defines whether it has tooltip content and how that content is built
    TooltipManager	Owns one tooltip, builds its contents, tracks its owner, shows/hides it
    UISystem	    Determines current hover and system conditions under which tooltip may appear
    Tooltip	        Concrete widget used to present tooltip contents
    UIRenderer	    Renders the tooltip like any other widget

The key rule is:
    Widgets provide tooltip content; 
    UISystem decides when a tooltip is appropriate; 
    TooltipManager manages the tooltip presentation.


20. Design Invariants
-----------------------------------------------------------------------------------------------------------------
The system relies on the following invariants:
    - There is exactly one Tooltip instance per TooltipManager.
    - TooltipManager owns the tooltip instance.
    - A widget may or may not provide tooltip content.
    - Tooltip content belongs conceptually to the widget that provides it.
    - UISystem determines the currently hovered widget.
    - A captured mouse suppresses tooltip display.
    - Mouse down hides the tooltip.
    - Changing tooltip owner clears the previous tooltip contents before rebuilding.
    - Tooltip is not part of the layer stack.
    - Widget does not depend on the concrete Tooltip class.
    - TooltipManager is the only component that directly manages the concrete tooltip instance.
    - Tooltip display is transient; the tooltip is rebuilt when its owner changes.


21. Overall Design
-----------------------------------------------------------------------------------------------------------------
The final architecture can be viewed as three distinct responsibilities:

                 ┌─────────────────────┐
                 │       UISystem      │
                 │                     │
                 │ MouseMove           │
                 │ MouseDown           │
                 │ mouseOver           │
                 │ mouseCapture        │
                 │                     │
                 │ "Should tooltip     │
                 │  be shown?"         │
                 └──────────┬──────────┘
                            │
                            │ Show(hover)
                            ▼
                 ┌─────────────────────┐
                 │   TooltipManager    │
                 │                     │
                 │ owns Tooltip        │
                 │ tracks owner        │
                 │ builds contents     │
                 │ shows/hides         │
                 │                     │
                 │ "How do I manage    │
                 │  the tooltip?"      │
                 └──────────┬──────────┘
                            │
                            │ BuildTooltip()
                            ▼
                 ┌─────────────────────┐
                 │       Widget        │
                 │                     │
                 │ SetTooltip(...)     │
                 │ tooltip builder     │
                 │                     │
                 │ "What is my         │
                 │  tooltip content?"  │
                 └─────────────────────┘

This keeps tooltip behavior out of the core widget event model while still allowing every widget to provide custom tooltip content.
The tooltip is therefore best understood not as a special behavior implemented by widgets, 
but as a system-managed presentation service that consumes an optional capability provided by widgets.


***************************************************************************************************************/
#pragma endregion

#pragma region // include files
#include <GUI/Widget.h>
#pragma endregion

using namespace engine::gui;

namespace engine
{
	namespace gui
	{
#pragma region // Tooltip

        // this class is a widget that represents a tooltip. it is used to display tooltip information for other widgets.
        // it will contain UI tree that is tooltip content of a widget
        // it is not movable, not focusable, and not droppable. it is always drawn on top of other widgets. it is managed by TooltipManager.
        class Tooltip : public Widget
        {
        private:
            // only TooltipManager can create and manipulate the tooltip. 
            friend class TooltipManager;

        protected:
            // tooltip is not movable, not focusable, not droppable, and will never be hit
            Tooltip();

            // handle rendering
            void Draw(const UIDrawContext& context) const override;
        };
#pragma endregion

#pragma region // TooltipManager
        // this class owns and manages the lifecycle of an actual tooltip instance and controls how tooltip content is presented
        // it does not decide whether to show or hide a tooltip, but if it's told to show a tooltip, it will show it his way
        class TooltipManager
        {
        private:
            // tooltip widget.
            Tooltip m_tooltip;

            // owner of the current tooltip if there is any
            Widget* m_owner;

        public:
            TooltipManager();
            void Hide();
            void Show(Widget* hover);
            const Widget* Get() const;
        };
#pragma endregion
	}
}