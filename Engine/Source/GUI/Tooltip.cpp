#include <GUI/Tooltip.h>
#include <GUI/gui.h>


#pragma region // Tooltip
namespace engine
{
	namespace gui
	{
#pragma region // Tooltip

		// tooltip is not movable, not focusable, not droppable, and will never be hit
		Tooltip::Tooltip()
		{
			m_moveBehavior = MoveBehavior::None;
			m_focusable = false;
			m_droppable = false;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		// handle rendering
		void Tooltip::Draw(const UIDrawContext& context) const
		{
			if (context.skin) context.skin->DrawTooltip(*this, context);
		}
#pragma endregion

#pragma region // TooltipManager
		TooltipManager::TooltipManager() :
			m_owner(nullptr)
		{
			m_tooltip.SetPosition({ 0,0 });
			m_tooltip.SetSize({ 0,0 });
		}

		void TooltipManager::Hide()
		{
			// clear the tooltip, hide it, and dereference owner widget
			m_tooltip.RemoveChildren();
			m_tooltip.SetPosition({ 0,0 });
			m_tooltip.SetSize({ 0,0 });
			m_tooltip.Hide();
			m_owner = nullptr;
		}

		void TooltipManager::Show(Widget* hover)
		{
			// bail out if this widget has no tooltip
			if (!hover || !hover->HasTooltip())
			{
				Hide();
				return;
			}

			// if this widget is not same as current owner, replace it with this widget
			if (hover != m_owner)
			{
				// do this first to flush the old tooltip
				Hide();

				// rebuild tooltip for new owner
				hover->BuildTooltip(m_tooltip);

				// this is new tooltip owner now
				m_owner = hover;

				m_tooltip.SetPosition(m_tooltip.GetPosition() + m_owner->GetAbsolutePosition());
			}

			// show tooltip
			m_tooltip.Show();
		}

		const Widget* TooltipManager::Get() const
		{
			return &m_tooltip;
		}
#pragma endregion
	}
}
#pragma endregion