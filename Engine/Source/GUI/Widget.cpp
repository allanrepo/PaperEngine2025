#include <GUI/gui.h>


#pragma region // Widget
namespace engine
{
	namespace gui
	{

#pragma region // DragController
		Widget::DragHandler::DragHandler(Widget* widget)
			: m_widget(widget)
		{
		}

		void Widget::DragHandler::Begin(const PositionF& position)
		{
			// if not movable, bail out
			if (m_widget->m_moveBehavior == MoveBehavior::None) return;

			// remember this mouse position. this will be the pivot position as this widget gets dragged around by mouse
			m_beginMousePosition = position;

			// remember the widget's position now. this will be the reference position as it gets dragged around by mouse
			m_beginMovePosition = m_widget->GetPosition();

			// this widget is now moving
			m_isMoving = true;

			m_widget->DragBegin(DragEventArgs{ GetBeginPosition(), position });
		}

		void Widget::DragHandler::Update(const PositionF& position)
		{
			if (m_isMoving)
			{
				// calculate the mouse movement delta between its position at start of mouse drag and its position now
				// factor in the move state - free? horizontal? vertical?
				VecF delta =
				{
					// if we can move horizontally, use the mouse position. otherwise, use begin position
					(m_widget->m_moveBehavior & MoveBehavior::Horizontal) ? position.x - m_beginMousePosition.x : 0.0f,

					// if we can move vertically, use the mouse position. otherwise, use begin position
					(m_widget->m_moveBehavior & MoveBehavior::Vertical) ? position.y - m_beginMousePosition.y : 0.0f
				};

				// we only drag the widget if new position after this drag is different from current position
				PositionF dragpos = m_beginMovePosition + delta;
				if (dragpos == m_widget->GetPosition()) return;

				// transform widget's position based on mouse movement delta
				m_widget->SetPosition(m_beginMovePosition + delta);

				m_widget->DragMove(DragEventArgs{ GetBeginPosition(), position });
			}
		}

		void Widget::DragHandler::End(const PositionF& position)
		{
			// if we never moved, we shouldn't be ending move. we shouldn't be firing up DragEnd event
			if (!m_isMoving) return;

			m_isMoving = false;

			m_widget->DragEnd(DragEventArgs{ GetBeginPosition(), position });
		}

		bool Widget::DragHandler::IsDragging() const
		{
			return m_isMoving;
		}

		PositionF Widget::DragHandler::GetBeginPosition() const
		{
			return m_beginMousePosition;
		}
#pragma endregion

#pragma region // hooks/callbacks for all actions that widget performs. derived class can override these to react to events
		// callback hook for registering this widget to system
		bool Widget::OnRegisterToSystem()
		{
			return true;
		}

		// callback hook for unregistering this widget from system
		bool Widget::OnUnregisterToSystem()
		{
			return true;
		}

		// callback hook for when this widget's position has changed
		void Widget::OnPositionChanged(const PositionF& oldPos, const PositionF& newPos)
		{
		}

		// callback hook for when this widget's size has changed
		void Widget::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
		}

		// callback hook for when this widget's resources have changed, e.g. font change
		void Widget::OnResourceChange()
		{
		}

		// callback hook for when this widget is mouse down. derived class can override this to react to mouse down event
		void Widget::OnMouseDown(const PositionF& position)
		{
		}

		// callback hook for when this widget is mouse up. derived class can override this to react to mouse up event
		void Widget::OnMouseUp(const PositionF& position)
		{
		}

		// callback hook for when mouse is moved over this widget. derived class can override this to react to mouse move event
		void Widget::OnMouseMove(const PositionF& position)
		{
		}

		// callback hook for when mouse moves into this widget. derived class can override this to react to mouse enter event
		void Widget::OnMouseEnter()
		{
		}

		// callback hook for when mouse moves out of this widget. derived class can override this to react to mouse leave event
		void Widget::OnMouseLeave()
		{
		}

		// callback hook for when key is pressed down while this widget has focus. derived class can override this to react to key down event
		void Widget::OnKeyDown(int key)
		{
		}

		// callback hook for when key is released while this widget has focus. derived class can override this to react to key up event
		void Widget::OnKeyUp(int key)
		{
		}

		// callback hook for when this widget has gained focus. derived class can override this to react to focus event
		void Widget::OnGotFocus()
		{
		}

		// callback hook for when this widget has lost focus. derived class can override this to react to lost focus event
		void Widget::OnLostFocus()
		{
		}

#pragma endregion

#pragma region // internal methods for tree traversal and z order

		// traverse through the tree and find the top-most widget that intersects with point. 
		// starts from this widget and traverses down the tree. returns nullptr if no widget intersects with point
		Widget* Widget::FindTopWidgetAt(const PositionF& position, unsigned int flag)
		{
			// if widget is hidden, bail out
			if (!IsVisible() && (flag & SearchFlags::Visible))
			{
				return nullptr;
			}

			// if widget is disabled, bail out
			if (!IsEnabled() && (flag & SearchFlags::Enabled))
			{
				return nullptr;
			}

			// if widget is not focusable, bail out
			if (!IsFocusable() && (flag & SearchFlags::Focusable))
			{
				return nullptr;
			}

			// do self test first. if this widget did not intersect with point, none of the children can. bail out
			if (!Contains(position))
			{
				return nullptr;
			}

			for (std::vector<std::unique_ptr<Widget>>::reverse_iterator it = m_children.rbegin(); it != m_children.rend(); it++)
			{
				// find the top widget at this child. this call will also check this child for intersect
				Widget* hit = it->get()->FindTopWidgetAt(position, flag);
				if (hit) return hit;
			}

			// if none of this widget's children intersect with point, then this widget does
			return this;
		}

		// find the top child that is visible, enabled, and intersects with given point
		Widget* Widget::FindTopChildAt(const PositionF& position, int flag)
		{
			for (std::vector<std::unique_ptr<Widget>>::reverse_iterator it = m_children.rbegin(); it != m_children.rend(); it++)
			{
				// if widget is hidden, bail out
				if (!(*it)->IsVisible() && (flag & SearchFlags::Visible))
				{
					continue;
				}

				// if widget is disabled, bail out
				if (!(*it)->IsEnabled() && (flag & SearchFlags::Enabled))
				{
					continue;
				}

				// if widget is not focusable, bail out
				if (!(*it)->IsFocusable() && (flag & SearchFlags::Focusable))
				{
					continue;
				}

				// if this widget intersects with point..
				if ((*it)->Contains(position))
				{
					// note we're returning this child, not this child's possible descendants that might have intersected with the point 
					return it->get();
				}
			}

			// returns nullptr if none of this widget's children intersects with point
			return nullptr;
		}

		// find the top child that intersects with given point and bring it to front. this is used for z order handling. 
		// returns the top-most widget that intersects with point
		Widget* Widget::FindAndResolveZOrderAt(const PositionF& position, int flag)
		{
			Widget* widget = this;

			// if widget is hidden, bail out
			if (!widget->IsVisible() && (flag & SearchFlags::Visible))
			{
				return nullptr;
			}

			// if widget is disabled, bail out
			if (!widget->IsEnabled() && (flag & SearchFlags::Enabled))
			{
				return nullptr;
			}

			// if widget is not focusable, bail out
			if (!widget->IsFocusable() && (flag & SearchFlags::Focusable))
			{
				return nullptr;
			}

			// check first if point is inside the root. bail out if not.
			if (!widget->Contains(position))
			{
				return nullptr;
			}

			while (true)
			{
				// returns nullptr if none of the widget's child intersects with p
				Widget* child = widget->FindTopChildAt(position, flag);

				// bring the child to front is not really part of routing. this is z order handling
				// but its convenient here. the right way architecturally is to collect route path 
				// then process the route path outside of routing. however, that may introduce unnecessary
				// performance impact so doing z order handling here is the best.				
				if (child)
				{
					widget->BringChildToFront(child);
				}
				else
				{
					break;
				}

				widget = child;
			}

			return widget;
		}

		// checks if this widget is descendant of given widget
		bool Widget::IsDescendantOf(const Widget* ancestor) const
		{
			if (!ancestor)
			{
				return false;
			}

			const Widget* current = m_parent;

			// traverse through the parents until given widget is found or root is reached
			while (current)
			{
				if (current == ancestor)
				{
					return true;
				}

				current = current->m_parent;
			}

			// if you reached this point, this widget is not descendant of given widget
			return false;
		}

		// bring the given child to front of this widget's children list. this is used for z order handling
		void Widget::BringChildToFront(Widget* child)
		{
			// use find_if better than for loop because you iterator on erase()
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[&](const auto& ptr)
				{
					return ptr.get() == child;
				});

			// no child? bail out
			if (it == m_children.end())
				return;

			// move this widget out of the children's list
			std::unique_ptr<Widget> node = std::move(*it);
			m_children.erase(it);

			// put it back at the end of the children's list so it will be at the front
			m_children.push_back(std::move(node));
		}

		// bring this widget to front of its parent's children list. this is used for z order handling
		void Widget::BringToFront()
		{
			// if this has no parent, then it has no siblings. then it does not have z order
			if (!m_parent) return;

			m_parent->BringChildToFront(this);
		}
#pragma endregion

#pragma region // system registration - internal methods called when widget is added to or removed from UI tree	
		bool Widget::UnregisterToSystem()
		{
			// callback hook for derived class to do its own cleanup before detaching from system
			OnUnregisterToSystem();

			// this will remove this widget from UISystem's focus, mouse capture, and mouse over tracking if this widget is currently being tracked by UISystem
			UISystem* system = GetSystem();
			if (system) system->Detach(this);
			return true;
		}

		bool Widget::RegisterToSystem()
		{
			return OnRegisterToSystem();
		}
#pragma endregion

#pragma region // hit test
		bool Widget::Contains(const PositionF& position) const
		{
			// always pass 
			if (m_hitTestBehavior == HitTestBehavior::AlwaysPass) return true;

			// always fail
			if (m_hitTestBehavior == HitTestBehavior::AlwaysFail) return false;

			// translate the point (assume to be absolute position) into this widget's local space
			PositionF local = position - GetAbsolutePosition();

			// convert our size into rect. 
			RectF rect{ 0, 0, m_size.width, m_size.height };

			// since point is now in widget's local space, we can check if its inside it
			return rect.Contains(local);
		}
#pragma endregion

#pragma region // heirarchy management
		void Widget::AddChild(std::unique_ptr<Widget> child)
		{
			child->m_parent = this;

			Widget* c = child.get();
			m_children.push_back(std::move(child));

			// traverse through this widget's whole tree including itself and register them to system
			c->ForEachWidget([&](Widget* widget)
				{
					widget->RegisterToSystem();
					return true;
				});
		}

		void Widget::RemoveChild(Widget* widget)
		{
			// is this widget our child?
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[&](const auto& ptr)
				{
					return ptr.get() == widget;
				});

			// unregister this widget's whole tree including itself. then remove this widget
			if (it != m_children.end())
			{
				widget->ForEachWidget([&](Widget* w)
					{
						w->UnregisterToSystem();
						return true;
					});

				m_children.erase(it);
			}
		}

		void Widget::RemoveChildren()
		{
			// remove all children and unregister their trees from system
			while (m_children.size())
			{
				m_children.back()->ForEachWidget([&](Widget* w)
					{
						w->UnregisterToSystem();
						return true;
					});

				m_children.pop_back();
			}
		}

		Widget* Widget::GetParent() const
		{
			return m_parent;
		}

		// remove a widget in this widget tree. this will traverse through this widget's tree to find the widget
		// if found, removes it as well as its tree. returns true if successfully found and removed
		bool Widget::Remove(Widget* widget)
		{
			Widget* found = nullptr;

			//bool result = false;
			// do not remove self, so just start searching from children onwards
			for (const std::unique_ptr<Widget>& child : m_children)
			{
				// no need to continue searching children if we already found the widget we're looking for
				if (found) break;

				child->ForEachWidget([&](Widget* w)
					{
						if (w == widget)
						{
							// found the widget we're looking for
							found = w;

							// return false to tell foreach to stop traversing now
							return false;
						}

						// tell foreach to continue traversing
						return true;
					});
			}

			// if we didn't find widget...
			if (!found) return false;

			// be strict here. ensure this widget has parent so we can remove it
			if (!found->m_parent)
			{
				throw std::runtime_error("how come this widget has no parent and is getting remove?");
			}

			// time to safely remove the widget
			found->m_parent->RemoveChild(found);
			return true;
		}

		bool Widget::HasChildren() const
		{
			return m_children.size() > 0;
		}

		// move a child widget to a new parent. this will remove the child from its current parent and add it to the new parent
		void Widget::MoveChildTo(Widget* child, Widget* newParent)
		{
			// we're a bit strict here
			if (!child)
			{
				throw std::invalid_argument("MoveChildTo() - child is null");
			}

			// we're a bit strict here
			if (!newParent)
			{
				throw std::invalid_argument("MoveChildTo() - newParent is null");
			}

			// make sure child is not same as new parent
			if (child == newParent)
			{
				throw std::invalid_argument("MoveChildTo() - newParent is same as child");
			}

			// be more strict. new parent cannot be descendant of child
			if (newParent->IsDescendantOf(child))
			{
				throw std::invalid_argument("MoveChildTo() - newParent is descendant of child");
			}

			// child must belong to this parent
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[child](const std::unique_ptr<Widget>& ptr)
				{
					return ptr.get() == child;
				});

			// we're a bit strict here
			if (it == m_children.end())
			{
				throw std::runtime_error("MoveChildTo() - child not found");
			}

			// transfer ownership out of current parent
			std::unique_ptr<Widget> movedChild = std::move(*it);

			// remove empty slot
			m_children.erase(it);

			// unregister this widget's whole tree including itself. 
			// once this child is added to new parent, new parent will register this child and its tree again to system
			movedChild->ForEachWidget([&](Widget* widget)
				{
					widget->UnregisterToSystem();
					return true;
				});

			// add to new parent
			newParent->AddChild(std::move(movedChild));
		}
#pragma endregion

#pragma region // state management
		void Widget::Show()
		{
			m_visible = true;
		}

		void Widget::Hide()
		{
			m_visible = false;
		}

		bool Widget::IsVisible() const
		{
			return m_visible;
		}

		void Widget::Enable()
		{
			m_enabled = true;
		}

		void Widget::Disable()
		{
			m_enabled = false;
		}

		bool Widget::IsEnabled() const
		{
			// if this widget is disabled, can return now
			if (!m_enabled) return false;

			// widgets has dependency on their parents/ascendants when it comes to enable state
			// if parent is disabled, then this must be disabled too.
			if (m_parent) return m_parent->IsEnabled();

			// if this is enabled as well as its ascendants, then this is enabled
			return true;
		}

#pragma endregion

#pragma region // behavior management
		bool Widget::IsFocusable() const
		{
			return m_focusable;
		}

		bool Widget::IsDroppable() const
		{
			return m_droppable;
		}
#pragma endregion

#pragma region // transform and extent properties
		float Widget::GetWidth() const
		{
			return m_size.width;
		}

		float Widget::GetHeight() const
		{
			return m_size.height;
		}

		SizeF Widget::GetSize() const
		{
			return m_size;
		}

		void Widget::SetSize(const SizeF& size)
		{
			// if size did not change, no need to update and invoke events
			if (m_size == size) return;

			SizeF oldSize = m_size;
			m_size = size;
			Resized(size);
			OnSizeChanged(oldSize, size);
		}

		PositionF Widget::GetAbsolutePosition() const
		{
			PositionF position = m_position;
			if (m_parent)
			{
				position += m_parent->GetAbsolutePosition();
			}
			return position;
		}

		void Widget::SetPosition(const PositionF& pos)
		{
			// if position did not change, no need to update and invoke events
			if (m_position == pos) return;

			PositionF oldPos = m_position;
			m_position = pos;
			Moved(pos);
			OnPositionChanged(oldPos, m_position);
		}

		PositionF Widget::GetPosition() const
		{
			return m_position;
		}

		RectF Widget::GetAbsoluteRect() const
		{
			PositionF absPos = GetAbsolutePosition();
			SizeF size = GetSize();
			return RectF
			{
				absPos.x,
				absPos.y,
				absPos.x + size.width,
				absPos.y + size.height
			};
		}
#pragma endregion

#pragma region // input handlers. these are called by the UI system when input events are received.
		void Widget::MouseDown(const PositionF& position)
		{
			// let derived widget handle mouse down event first
			OnMouseDown(position);

			m_dragHandler.Begin(position);
		}

		void Widget::MouseUp(const PositionF& position)
		{
			m_dragHandler.End(position);

			// now we handle mouse up event after we set its to state to NOT moving
			OnMouseUp(position);
		}

		void Widget::MouseMove(const PositionF& position)
		{
			m_dragHandler.Update(position);

			// handle this mouse event after this widget updates its position from mouse move
			OnMouseMove(position);
		}

		void Widget::MouseEnter()
		{
			OnMouseEnter();
		}

		void Widget::MouseLeave()
		{
			OnMouseLeave();
		}

		void Widget::KeyDown(int key)
		{
			OnKeyDown(key);
		}

		void Widget::KeyUp(int key)
		{
			OnKeyUp(key);
		}
#pragma endregion

#pragma region // tooltip support
		bool Widget::HasTooltip() const
		{
			return m_tooltipBuilder != nullptr;
		}

		void Widget::BuildTooltip(Widget& tooltip)
		{
			if (m_tooltipBuilder)
			{
				m_tooltipBuilder(*this, tooltip);
			}
		}

		void Widget::SetTooltip(std::function<void(Widget&, Widget&)> builder)
		{
			m_tooltipBuilder = std::move(builder);
		}
#pragma endregion

#pragma region // resource management
		void Widget::ResourceChange()
		{
			OnResourceChange();
		}
#pragma endregion
	}
}
#pragma endregion