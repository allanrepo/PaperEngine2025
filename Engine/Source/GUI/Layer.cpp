#include <GUI/gui.h>

#pragma region // Layer
namespace engine
{
	namespace gui
	{
		// overriden from Widget to return the UISystem as Layer is a root widget and has no parent.  
		UISystem* Layer::GetSystem() const
		{
			return m_system;
		}

		Layer::Layer(UISystem* system, Widget* owner, const PositionF& pos, const SizeF& size, const Type& type, bool movable) :
			m_owner(owner),
			m_system(system),
			m_type(type)
		{
			// can be movable or not. 
			m_moveBehavior = movable ? Widget::MoveBehavior::Free : Widget::MoveBehavior::None;

			// set the position and size of the layer. this is usually done by the builder function, but can be set here as well.
			SetPosition(pos);
			SetSize(size);

			// layer is not focusable. it is a root widget and responsible for the extent of its children
			m_focusable = false;
		}

		Widget* Layer::GetOwner() const
		{
			return m_owner;
		}

		bool Layer::IsModal() const
		{
			return m_type == Type::Modal;
		}

		bool Layer::IsMenu() const
		{
			return m_type == Type::Menu;
		}

		bool Layer::IsPopup() const
		{
			return m_type == Type::Popup;
		}

		Layer::Type Layer::GetType() const
		{
			return m_type;
		}

		void Layer::Draw(const UIDrawContext& context) const
		{
			if (context.skin) context.skin->DrawLayer(*this, context);
		}
	}
}
#pragma endregion