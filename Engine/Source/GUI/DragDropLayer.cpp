#include <GUI/DragDropLayer.h>
#include <GUI/gui.h>


#pragma region // Drag and Drop Feature
namespace engine
{
	namespace gui
	{
#pragma region // DragDropLayer

		UISystem* DragDropLayer::GetSystem() const 
		{
			return m_system;
		}

		DragDropLayer::DragDropLayer(UISystem* system) :
			m_system(system)
		{
			// this layer should not be movable at all. it should remain and behave like a root widget
			m_moveBehavior = MoveBehavior::None;

			// not focusable, not droppable
			m_focusable = false;
			m_droppable = false;

			// not necessary but just making it explicit to tell that this layer is root like
			SetPosition({ 0,0 });
		}

		// this method is called when a drag/drop state on a given widget is about to begin
		void DragDropLayer::Begin(Widget* draggable)
		{
			// only reason why our draggables contain something is if we previously started dragging a draggable and has not dropped it yet.
			// starting another drag while in this state is unacceptable. it should not happen
			if (m_draggables.Size())
			{
				throw std::runtime_error("we're about to start dragging something, why are we already in dragging state?");
			}

			// just to be sure, layer should not have any children before we begin a drag. if it does, it means it is dragging something already
			// so that is not possible. 
			if (HasChildren())
			{
				throw std::runtime_error("we're about to start dragging something, why do we already dragging something?");
			}

			// it's not possible to drag an invalid draggable widget. 
			if (!draggable)
			{
				throw std::runtime_error("draggable widget cannot be invalid");
			}

			// let's be strict here. if dragged widget has no parent, that is not acceptable!
			if (!draggable->GetParent())
			{
				throw std::runtime_error("widget is not attached to any parent");
			}

			// save the absolute position of the drag widget. we need to translate its position once we move it to the layer 
			PositionF pos = draggable->GetAbsolutePosition();

			// save reference to original parent. in case target drop is not a valid droppable widget, this widget returns to its original parent
			DragDropContext context{};
			context.originalParent = draggable->GetParent();

			// also save reference to drag widget's original position relative to its original parent. 
			// in case target drop is not a valid droppable widget, drag widget remains in original parent and in original position
			context.originalPosition = draggable->GetPosition();

			// let's register this drag widget and its information so we remember later once we drop it
			m_draggables.Register(draggable, context);

			// let's now move the drag widget into the drag layer
			draggable->GetParent()->MoveChildTo(draggable, this);

			// we're policing very strictly here. may not be necessary, but good to have. also, we don't execute this every frame so i think it's ok to be strict.
			if (draggable->GetParent() != this)
			{
				throw std::runtime_error("failed to move draggable into dragdrop layer");
			}

			// since the drag widget is now a child of this layer, let's translate its position from absolute to relative to this layer
			// note that dragdrop layer is root like and its position is 0,0 so translating does not do anything. but for now we do this to be explicit
			pos = pos - GetAbsolutePosition();
			draggable->SetPosition(pos);
		}

		void DragDropLayer::End(Widget* draggable, Widget* newParent)
		{
			// we're about to end dragging but if there is no draggable, how is this possible? this cannot happen
			if (!m_draggables.Size())
			{
				throw std::runtime_error("we're about to end dragging something, where are the draggables to drop?");
			}

			// our layer has no draggable to drop while trying to end a drag state? that is not possible
			if (!HasChildren())
			{
				throw std::runtime_error("we're about to end dragging something, why do we not contain a draggable?");
			}

			// it's not possible to drop an invalid draggable widget. 
			if (!draggable)
			{
				throw std::runtime_error("draggable widget cannot be invalid");
			}

			// if we're trying to drop a draggable that is not being dragged, something is wrong
			if (!m_draggables.Has(draggable))
			{
				throw std::runtime_error("trying to drop a draggable that is not tracked");
			}

			DragDropContext& context = m_draggables.Get(draggable);

			// check what's gonna be the parent - original or new?
			Widget* parent = newParent ?	// is new parent valid?
				newParent->IsDroppable() ?	// is new parent droppable?
				newParent :					// new parent is valid, set it
				context.originalParent :	// new parent is not droppable, so using the original parent
				context.originalParent;		// new parent is invalid, so using the original parent 

			// check what will be the position of the drag widget once it is dropped - is it back to original position or now in the new parent?
			// NOTE: we calculate position here before moving child to new parent because we refer to drag widget's absolute position here prior to being moved to new parent
			PositionF pos = newParent ?													// is new parent valid?
				newParent->IsDroppable() ?												// is new parent droppable?
				draggable->GetAbsolutePosition() - parent->GetAbsolutePosition() :		// new parent is valid, so position is now relative to new parent
				context.originalPosition :												// new parent is not droppable, so using original position
				context.originalPosition;												// new parent is invalid, so using original position

			// move the drag widget to new parent. either drop it on new parent, or return it back to original parent
			MoveChildTo(draggable, parent);

			// we're policing very strictly here. may not be necessary, but good to have. also, we don't execute this every frame so i think it's ok to be strict.
			if (draggable->GetParent() != parent)
			{
				throw std::runtime_error("failed to move draggable into a droppable parent");
			}

			// move position of the drag widget now relative to new parent
			draggable->SetPosition(pos);

			// TODO: do we need to notify anyone when drop happened? not sure yet. if we do, we should do it here.

			// clear our draggables list
			m_draggables.Clear();
		}
	}
#pragma endregion

#pragma region // Draggable
	// this widget is draggable via mouse move so we handle start of dragging through mouse down
	void Draggable::OnMouseDown(const PositionF& position)
	{
		UISystem* system = GetSystem();
		if (!system)
		{
			throw std::runtime_error("widget is not attached to any UISystem");
		}

		// let system know we want to drag this widget
		system->BeginDrag(this);
	}

	// this widget drops on mouse up
	void Draggable::OnMouseUp(const PositionF& position)
	{
		UISystem* system = GetSystem();
		if (!system)
		{
			throw std::runtime_error("widget is not attached to any UISystem");
		}

		// let system know we want this widget to drop
		system->EndDrag(this, position);
	}

	Draggable::Draggable()
	{
		m_moveBehavior = Widget::MoveBehavior::Free;
	}

	void Draggable::Draw(const UIDrawContext& context) const 
	{
		if (context.skin) context.skin->DrawDraggable(*this, context);
	}

#pragma endregion
}
#pragma endregion