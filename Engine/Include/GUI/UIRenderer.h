#pragma once
#pragma region // Design Document
/****************************************************************************************************************
# UI Rendering & Skin Design

-----------------------------------------------------------------------------------------------------------------
1. Purpose
-----------------------------------------------------------------------------------------------------------------
The UI rendering system is responsible for translating the logical state of widgets into visual output.
The UI system owns widget hierarchy, interaction, focus, capture, layers, layout, and input routing. 
Rendering is kept separate from these responsibilities.

The rendering design is based on two concepts:

    Widget — owns the logical/UI state.
    UISkin — knows how each supported widget type should be visually rendered.

The widget does not directly contain rendering implementation.

-----------------------------------------------------------------------------------------------------------------
2. Design
-----------------------------------------------------------------------------------------------------------------
The core interface is UISkin.

Each widget type delegates its visual representation to the skin.

Conceptually:

    Widget
       │
       │ logical state
       ▼
    UISkin
       │
       │ visual interpretation
       ▼
    UI rendering backend


The widget describes what it is and what state it is in.
The skin decides how that state looks.

-----------------------------------------------------------------------------------------------------------------
3. UISkin Responsibilities
-----------------------------------------------------------------------------------------------------------------
UISkin provides the visual interpretation of widget types.

A skin may determine:
    - background appearance
    - borders
    - fonts
    - colors
    - textures
    - spacing
    - visual states
    - icons
    - animation parameters
    - widget-specific visual composition

For example:

    cpp
    class DarkSkin : public UISkin
    {
    public:
        void DrawButton(
            const Button& button,
            const UIDrawContext& ctx) const override;

        void DrawSlider(
            const Slider& slider,
            const UIDrawContext& ctx) const override;
    };


A different skin can render the same logical widget differently without modifying the widget itself.

-----------------------------------------------------------------------------------------------------------------
4. Type-Oriented Rendering
-----------------------------------------------------------------------------------------------------------------
The skin interface is intentionally type-oriented.

Each significant widget type has a corresponding drawing operation:

    Button       → DrawButton()
    Slider       → DrawSlider()
    ScrollBar    → DrawScrollBar()
    ListBox      → DrawListBox()
    Inventory    → DrawInventory()
    ...

This is intentional.

A new widget type is expected to potentially require a new rendering operation in UISkin.

For example, introducing:

    class Inventory : public Widget
    {
        ...
    };


may require:

    virtual void DrawInventory(
        const Inventory& inventory,
        const UIDrawContext& ctx) const = 0;


This explicit coupling is desirable because the skin must make an explicit decision about how the new widget is rendered.

-----------------------------------------------------------------------------------------------------------------
5. Adding a New Widget Type
-----------------------------------------------------------------------------------------------------------------
Adding a widget follows this general process:

    1. Create the widget
           ↓
    2. Define its logical state/API
           ↓
    3. Add its DrawXxx() operation to UISkin
           ↓
    4. Implement that operation in each concrete skin
           ↓
    5. Add the widget to the UI rendering traversal


The compiler helps identify concrete skins that have not yet implemented the new rendering operation.
This provides a useful completeness check.

-----------------------------------------------------------------------------------------------------------------
6. UIDrawContext
-----------------------------------------------------------------------------------------------------------------
UIDrawContext provides rendering information that is independent of the particular widget.

Examples may include:
    - renderer access
    - clipping rectangle
    - transformation
    - current layer information
    - viewport information
    - rendering scale
    - accumulated opacity
    - other traversal/rendering state

Conceptually:

    DrawButton(button, ctx);


means:
    Render this button using its current logical state and the current rendering context.

The context should not become a general-purpose container for arbitrary widget state. Widget-specific state belongs to the widget.

-----------------------------------------------------------------------------------------------------------------
7. Rendering Traversal
-----------------------------------------------------------------------------------------------------------------
The UI system is responsible for determining what should be rendered and in what order.
The skin is responsible for determining how an individual widget is visually rendered.

Conceptually:
    
    UISystem
       │
       ├── Layer
       │     ├── Widget
       │     ├── Widget
       │     └── Widget
       │
       └── Layer
             └── Widget
                    │
                    ▼
                 UISkin
                    │
                    ▼
                 Renderer


This keeps rendering order and UI ownership separate from visual styling.
The existing layer system therefore remains responsible for ordering concerns such as:
    - popup ordering
    - modal ordering
    - menus/submenus
    - clipping hierarchy
    - visibility
    - layer collapse

The skin does not manage these concerns.

-----------------------------------------------------------------------------------------------------------------
 8. Skin Does Not Own Widgets
-----------------------------------------------------------------------------------------------------------------
A skin receives widgets as references:

    void DrawButton(
        const Button& button,
        const UIDrawContext& ctx) const;

The skin does not own the widget.
This preserves the existing ownership model of the UI system.
Widgets remain owned by the widget tree/layer hierarchy, while the skin is simply a renderer/style provider.

-----------------------------------------------------------------------------------------------------------------
9. Rendering Backend Independence
-----------------------------------------------------------------------------------------------------------------
UISkin should describe UI appearance rather than expose low-level rendering implementation 
throughout the widget system.

The intended separation is:
    Widget
      │
      │ logical UI
      ▼
    UISkin
      │
      │ visual description
      ▼
    UI Renderer / Rendering Backend
      │
      ▼
    Graphics API


The widgets therefore remain independent of DirectX-specific rendering operations.


-----------------------------------------------------------------------------------------------------------------
10. Why Explicit Draw Functions Are Preferred
-----------------------------------------------------------------------------------------------------------------
A generic operation such as:

    Draw(const Widget& widget);


would appear more extensible, but would move type identification and dispatch somewhere else.
For example, the implementation would eventually need to determine:

    Is this a Button?
    Is this a Slider?
    Is this a ScrollBar?
    Is this an Inventory?
    ...

The explicit interface instead makes the supported widget types visible in the API.

Advantages include:
    - strong typing
    - straightforward implementations
    - easy debugging
    - compiler-enforced completeness
    - no string/type-ID dispatch
    - no generic property bag
    - clear ownership of rendering behavior
    - easy implementation of multiple skins

The additional maintenance cost of adding a new DrawXxx() function is accepted as part of the design.

-----------------------------------------------------------------------------------------------------------------
11. Extensibility Rule
-----------------------------------------------------------------------------------------------------------------
The rendering interface is expected to change when new fundamental widget types are introduced.

This is acceptable.

The design does not attempt to achieve an abstraction where adding a new widget never requires modifying UISkin.

Instead, it optimizes for:

    Explicitness and maintainability over maximum extensibility.

A new widget is a meaningful addition to the UI vocabulary. 
Requiring the skin interface to acknowledge that new vocabulary is intentional.

-----------------------------------------------------------------------------------------------------------------
12. Overall Architectural Boundary
-----------------------------------------------------------------------------------------------------------------
The final responsibility split is:

    Widget
        - What am I? What is my logical state?

    UISystem / Layer System
        - Which widgets exist, where are they, and in what order are they processed/rendered?

    UISkin
        - How should this widget look?

    UI Renderer
        - How do I issue the actual rendering operations?

    Graphics Backend
        - How do those operations reach the graphics API?

 This separation keeps the UI architecture understandable while allowing the 
 visual implementation to evolve independently from widget behavior.

-----------------------------------------------------------------------------------------------------------------
13. Guiding Principle
-----------------------------------------------------------------------------------------------------------------
The rendering system should remain deliberately boring.

A widget should be able to say, in effect:

    skin.DrawButton(*this, ctx);

and then get out of the way.

The skin should contain the visual knowledge.
The UI system should contain the UI-management knowledge.
The renderer should contain the rendering knowledge.

When a new widget is introduced, explicitly extending UISkin is considered normal 
and desirable rather than something the architecture should try to eliminate.
****************************************************************************************************************/
#pragma endregion

#pragma region // include files
#include <Graphics/Renderer/IRenderer.h>

#pragma endregion

namespace engine
{
    namespace gui
    {
#pragma region // namespaces
        using IRenderer = engine::graphics::renderer::IRenderer;
#pragma endregion

#pragma region // forward declarations
        class UISystem;
        class LayerStack;
        class Widget;
        struct UIDrawContext;
        class Layer;
        class Frame;
        class Tooltip;
        class Draggable;
        class MenuButton;
        class SubMenuButton;
        class MenuItem;
        class Thumb;
        class Slider;
        class CheckBox;
        class RadioButton;
        class ScrollBar;
        class ResizeableFrame;
        class Grip;
        class ViewPort;
        class Content;
        class ScrollView;
        class UniformGrid;
        class Stack;
        class TextListBox;
        class TextList;
#pragma endregion

#pragma region // UISkin
        // interface  
        // provides the visual interpretation of widget types.
        // Each widget type delegates its visual representation to the skin.
        // The widget describes what it is and what state it is in. the skin decides how that state looks.
        // A different skin can render the same logical widget differently without modifying the widget itself.
        class UISkin
        {
        public:
            virtual ~UISkin() = default;

            virtual void DrawButton(const class Button& button, const UIDrawContext& ctx) const = 0;
            virtual void DrawLayer(const class Layer& overlay, const UIDrawContext& ctx) const = 0;
            virtual void DrawFrame(const class Frame& frame, const UIDrawContext& ctx) const = 0;
            virtual void DrawTooltip(const class Tooltip& tooltip, const UIDrawContext& ctx) const = 0;
            virtual void DrawLabel(const class Label& label, const UIDrawContext& ctx) const = 0;
            virtual void DrawImage(const class Image& image, const UIDrawContext& ctx) const = 0;
            virtual void DrawDraggable(const Draggable& draggable, const UIDrawContext& context) const = 0;
            virtual void DrawMenuButton(const MenuButton& menuButton, const UIDrawContext& context) const = 0;
            virtual void DrawMenuItem(const MenuItem& menuItem, const UIDrawContext& context) const = 0;
            virtual void DrawSubMenuButton(const SubMenuButton& subMenuButton, const UIDrawContext& context) const = 0;
            virtual void DrawSlider(const Slider& slider, const UIDrawContext& context) const = 0;
            virtual void DrawScrollBar(const ScrollBar& scrollbar, const UIDrawContext& context) const = 0;
            virtual void DrawThumb(const Thumb& thumb, const UIDrawContext& context) const = 0;
            virtual void DrawCheckBox(const CheckBox& checkbox, const UIDrawContext& context) const = 0;
            virtual void DrawRadioButton(const RadioButton& radiobutton, const UIDrawContext& context) const = 0;
            virtual void DrawGrip(const Grip& radiobutton, const UIDrawContext& context) const = 0;
            virtual void DrawResizeableFrame(const ResizeableFrame& radiobutton, const UIDrawContext& context) const = 0;
            virtual void DrawViewPort(const ViewPort& vp, const UIDrawContext& context) const = 0;
            virtual void DrawContent(const Content& content, const UIDrawContext& context) const = 0;
            virtual void DrawScrollView(const ScrollView& scrollview, const UIDrawContext& context) const = 0;
            virtual void DrawUniformGrid(const UniformGrid& grid, const UIDrawContext& context) const = 0;
            virtual void DrawStack(const Stack& stack, const UIDrawContext& context) const = 0;
            virtual void DrawTextListBox(const TextListBox& box, const UIDrawContext& context) const = 0;
            virtual void DrawTextList(const TextList& text, const UIDrawContext& context) const = 0;
        };
#pragma endregion

#pragma region // UIDrawContext
        // provides rendering information that is independent of the particular widget.
        struct UIDrawContext
        {
            IRenderer& renderer;
            UISystem& system;
            UISkin& skin;
        };
#pragma endregion

#pragma region // UIRenderer
        class UIRenderer
        {
        private:
        public:
            static void Draw(const UIDrawContext& context, const Widget& widget);
        };
#pragma endregion
    }
}