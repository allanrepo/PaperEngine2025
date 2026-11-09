#include <GUI/Widget.h>
#include <Utilities/Logger.h>


#pragma region // Widget
namespace engine
{
	namespace gui
	{
#pragma region // DragController
		Widget::DragTracker::DragTracker(Widget* widget)
			: m_widget(widget)
		{
		}

		void Widget::DragTracker::Begin(const PositionF& position)
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

		void Widget::DragTracker::Update(const PositionF& position)
		{
			if (m_isMoving)
			{

				// the position passed is the position after drag but this position does not care if this widget's movement is horizontal, vertical, or not moving at all.
				// so we calculate with the horizontal and vertical movement taken into consideration.
				PositionF currentPosition =
				{
					// if we can move horizontally, use the mouse position. otherwise, use begin position
					(m_widget->m_moveBehavior & MoveBehavior::Horizontal) ? position.x : m_beginMousePosition.x,

					// if we can move vertically, use the mouse position. otherwise, use begin position
					(m_widget->m_moveBehavior & MoveBehavior::Vertical) ? position.y : m_beginMousePosition.y
				};

				// calculate the mouse movement delta between its position at start of mouse drag and its position now
				// factor in the move state - free? horizontal? vertical?
				// since currentPosition already factored in move state, we just calculate delta as below
				VecF delta = currentPosition - m_beginMousePosition;

				// we only drag the widget if new position after this drag is different from current position
				PositionF dragpos = m_beginMovePosition + delta;
				if (dragpos == m_widget->GetPosition()) return;

				// transform widget's position based on mouse movement delta
				m_widget->SetPosition(m_beginMovePosition + delta);

				m_widget->DragMove(DragEventArgs{ GetBeginPosition(), currentPosition });
			}
		}

		void Widget::DragTracker::End(const PositionF& position)
		{
			// if we never moved, we shouldn't be ending move. we shouldn't be firing up DragEnd event
			if (!m_isMoving) return;

			m_isMoving = false;

			m_widget->DragEnd(DragEventArgs{ GetBeginPosition(), position });
		}

		bool Widget::DragTracker::IsDragging() const
		{
			return m_isMoving;
		}

		PositionF Widget::DragTracker::GetBeginPosition() const
		{
			return m_beginMousePosition;
		}
#pragma endregion

#pragma region // system reference
		// get system reference from the root of UI tree this widget belongs to. if this widget is not attached to any tree, it will return nullptr
		UISystem* Widget::GetSystem() const
		{
			if (m_parent)
			{
				return m_parent->GetSystem();
			}

			return nullptr;
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

		void Widget::OnVerbose(bool enable)
		{
		}

		// callback for when widget receives mouse wheel. returns false to propagate the event to its ancestor
		bool Widget::OnMouseWheel(const PositionF& screenPos, float delta)
		{
			return false;
		}

		// callback for when widget receives horizontal mouse wheel. returns false to propagate the event to its ancestor
		bool Widget::OnMouseHWheel(const PositionF& screenPos, float delta)
		{
			return false;
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

			// this is an event that allows UISystem to untrack this widget from its interaction states
			UnregisteredToSystem(this);

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

			// once the child widget is added, refresh its layout as well as its children.
			c->RefreshLayout();

			// if this child is anchored and its anchor is fill, then it will resize. if it resizes, its children, if any, must also refresh their layout.
			if (c->GetLayoutMode() == LayoutMode::Anchor && (c->GetVerticalAnchor() == VerticalAnchor::Fill || c->GetHorizontalAnchor() == HorizontalAnchor::Fill))
			{
				c->RefreshChildrenLayout();
			}
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

		bool Widget::IsDraggable() const
		{
			return m_moveBehavior == MoveBehavior::None ? false : true;
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
			OnSizeChanged(oldSize, size);

			// raise event to notify listeners that this widget's size has changed
			Resized(oldSize, size);

			// once this widget size changed and it fires up callback and notify listeners, 
			// refresh its position/size based on its layout and do so on its children
			RefreshLayout();
			RefreshChildrenLayout();
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

			// set to new position
			PositionF oldPos = m_position;
			m_position = pos;
			OnPositionChanged(oldPos, m_position);

			// raise event to notify listeners that this widget's position has changed
			Moved(oldPos, m_position);

			// once this widget is repositioned, we need to refresh its layout.
			// if its layout mode is anchored, then setting it to whatever new position is irrelevant
			// and it will reposition it again based on the layout mode.
			// its children does not need to their layouts to be refreshed since this widget's size won't change
			RefreshLayout();
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

			m_DragTracker.Begin(position);
		}

		void Widget::MouseUp(const PositionF& position)
		{
			m_DragTracker.End(position);

			// now we handle mouse up event after we set its to state to NOT moving
			OnMouseUp(position);
		}

		void Widget::MouseMove(const PositionF& position)
		{
			m_DragTracker.Update(position);

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

		void Widget::MouseWheel(const PositionF& screenPos, float delta)
		{
			// if callback returns true, it processed this event and does not wish to propagate to its ancestor
			if (OnMouseWheel(screenPos, delta)) return;

			// propagate to parent if can
			if (m_parent) m_parent->MouseWheel(screenPos, delta);
		}

		void Widget::MouseHWheel(const PositionF& screenPos, float delta)
		{
			// if callback returns true, it processed this event and does not wish to propagate to its ancestor
			if (OnMouseHWheel(screenPos, delta)) return;

			// propagate to parent if can
			if (m_parent) m_parent->MouseHWheel(screenPos, delta);
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

#pragma region // layout management
		void Widget::SetLayoutMode(LayoutMode mode)
		{
			// if already set to same mode, do nothing. no need to trigger refresh layout
			if (m_layoutMode == mode) return;

			m_layoutMode = mode;

			// because mode change, this widget might need to be repositioned or resized 
			// since it might be resized, we also need to refresh its children
			RefreshLayout();
			RefreshChildrenLayout();
		}

		LayoutMode Widget::GetLayoutMode() const
		{
			return m_layoutMode;
		}

		void Widget::SetHorizontalAnchor(HorizontalAnchor anchor)
		{
			// if already set to same anchor, do nothing. no need to trigger refresh layout
			if (m_horizontalAnchor == anchor) return;

			m_horizontalAnchor = anchor;

			// change in anchor only matters if our layout mode is anchored
			if (m_layoutMode != LayoutMode::Anchor) return;

			// refresh this widget's layout since anchor changed. 
			RefreshLayout();

			// if this widget is anchored and its anchor is fill, then it will resize. if it resizes, its children, if any, must also refresh their layout.
			if (GetLayoutMode() == LayoutMode::Anchor && (GetVerticalAnchor() == VerticalAnchor::Fill || GetHorizontalAnchor() == HorizontalAnchor::Fill))
			{
				RefreshChildrenLayout();
			}
		}

		void Widget::SetVerticalAnchor(VerticalAnchor anchor)
		{
			// if already set to same anchor, do nothing. no need to trigger refresh layout
			if (m_verticalAnchor == anchor) return;

			m_verticalAnchor = anchor;

			// change in anchor only matters if our layout mode is anchored
			if (m_layoutMode != LayoutMode::Anchor) return;

			// refresh this widget's layout since anchor changed. 
			RefreshLayout();

			// if this widget is anchored and its anchor is fill, then it will resize. if it resizes, its children, if any, must also refresh their layout.
			if (GetLayoutMode() == LayoutMode::Anchor && (GetVerticalAnchor() == VerticalAnchor::Fill || GetHorizontalAnchor() == HorizontalAnchor::Fill))
			{
				RefreshChildrenLayout();
			}
		}

		HorizontalAnchor Widget::GetHorizontalAnchor() const
		{
			return m_horizontalAnchor;			
		}

		VerticalAnchor Widget::GetVerticalAnchor() const
		{
			return m_verticalAnchor;
		}

		// resolves the position and size of this widget given its layout mode.
		void Widget::RefreshLayout()
		{
			switch (m_layoutMode)
			{			
				case LayoutMode::Free:
				{
					// do nothing. we're using the actual position set for this widget
					break;
				}

				case LayoutMode::Anchor:
				{
					// if this widget has no parent, we cannot anchor. bail out
					if (!m_parent) break;

					// let's get the size of the parent widget. this will be the region where this widget will be aligned and anchored into
					SizeF region = m_parent->GetSize();

					// get its initial position and size. we will modify the position based on the anchor properties
					PositionF pos = GetPosition();
					SizeF size = GetSize();

					switch (m_horizontalAnchor)
					{
					case HorizontalAnchor::Center:
						pos.x = region.width / 2.0f - m_size.width / 2.0f;
						break;
					case HorizontalAnchor::Left:
						pos.x = m_margin.left;
						break;
					case HorizontalAnchor::Right:
						pos.x = region.width - m_size.width - m_margin.right;
						break;
					case HorizontalAnchor::Fill:
						pos.x = m_margin.left;
						size.width = region.width - m_margin.left - m_margin.right;
						break;
					default:
						throw std::runtime_error("invalid horizontal anchor");
					}

					switch (m_verticalAnchor)
					{
					case VerticalAnchor::Center:
						pos.y = region.height / 2.0f - m_size.height / 2.0f;
						break;
					case VerticalAnchor::Top:
						pos.y = m_margin.top;
						break;
					case VerticalAnchor::Bottom:
						pos.y = region.height - m_size.height - m_margin.bottom;
						break;
					case VerticalAnchor::Fill:
						pos.y = m_margin.top;
						size.height = region.height - m_margin.top - m_margin.bottom;
						break;
					default:
						throw std::runtime_error("invalid vertical anchor");
					}

					// set the new position and size of this widget based on the anchor properties
					SetPosition(pos);
					SetSize(size);

					break;
				}
			}
		}

		void Widget::RefreshChildrenLayout()
		{
			for (auto& child : m_children)
			{
				child->RefreshLayout();
			}
		}

		void Widget::SetMargin(const RectF& margin)
		{
			// if margin is already set to the same value, do nothing. no need to trigger refresh layout
			if (m_margin == margin) return;

			m_margin = margin;

			// margin is only relevant if layout mode is anchored. if so, refresh this widget's layout
			if (m_layoutMode != LayoutMode::Anchor) return;

			// since margin changed, this widget might need to be repositioned or resized. if it resizes, its children must also refresh their layout.
			SizeF oldSize = m_size;

			// refresh this widget's layout since margin changed.
			RefreshLayout();

			// if this widget's size changed, refresh its children's layout as well
			if(oldSize != m_size)
			{
				RefreshChildrenLayout();
			}
		}

		RectF Widget::GetMargin() const
		{
			return m_margin;
		}

#pragma endregion

#pragma region // attributes
		void Widget::SetName(const std::string& name)
		{
			m_name = name;
		}

		std::string Widget::GetName() const
		{
			return m_name;
		}

		void Widget::Verbose(bool enable)
		{
			m_verbose = enable;

			OnVerbose(m_verbose);
		}

		bool Widget::IsVerbose() const
		{
			return m_verbose;
		}
#pragma endregion
	}
}
#pragma endregion