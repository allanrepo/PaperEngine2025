#include <GUI/UIRenderer.h>
#include <GUI/Widget.h>

namespace engine
{
    namespace gui
    {


#pragma region // UIRenderer
        void UIRenderer::Draw(const UIDrawContext& context, const Widget& widget)
        {
            // if widget is hidden, its whole tree is also hidden. bail out
            if (!widget.IsVisible()) return;

            // draw this widget
            widget.Draw(context);

            // get current clip region from renderer. intersect with this widget's rect to get effective clip region. 
            RectF orig = context.renderer.GetClipRegion();
            RectF effective = widget.GetAbsoluteRect().Intersect(orig);

            // apply effective clip region to renderer. this will make sure this widget's tree will be clipped by this widget's rect
            context.renderer.SetClipRegion(effective);

            // draw children
            widget.ForEachChild([&](Widget* widget)
                {
                    Draw(context, *widget);
                });

            // restore previous clip region after drawing this widget's tree
            context.renderer.SetClipRegion(orig);
        }
#pragma endregion
    }
}