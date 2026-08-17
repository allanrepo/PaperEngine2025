#include <Graphics/Renderer/IRenderer.h>
#include <Math/Size.h>
#include <Core/Event.h>
#include <Spatial/Position.h>
#include <Math/Rect.h>
#include <Containers/Dictionary.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/Sprite.h>
#include <Graphics/Core/Color.h>

#include <algorithm>
#include <Containers/Grid.h>

namespace engine
{
	namespace gui
	{
#pragma region // forward declaration
			class OverlayTrigger;
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

#pragma region // namespaces
		using IRenderer = engine::graphics::renderer::IRenderer;
		using PositionF = spatial::PositionF;
		using SizeF = math::SizeF;
		using VecF = engine::math::VecF;
		using RectF = engine::math::RectF;
		using IFontAtlas = engine::graphics::resource::IFontAtlas;
		using IRenderable = engine::graphics::IRenderable;
		using Sprite = engine::graphics::Sprite;
		using ColorF = engine::graphics::ColorF;

		template<typename K, typename T>
		using Dictionary = engine::container::Dictionary<K, T>;

		template<typename T>
		using Size = engine::math::Size<T>;

#pragma endregion

#pragma region // UISkin
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
		struct UIDrawContext
		{
			IRenderer& renderer;
			UISystem& system;
			UISkin* skin = nullptr;
			Widget* hover = nullptr;
			Widget* focus = nullptr;
			Widget* capture = nullptr;
		};
#pragma endregion

#pragma region // Widget
		class Widget
		{
		private:
#pragma region // DragController
			// --------------------------------------------------------------------------------
			// DRAG MANAGEMENT
			// --------------------------------------------------------------------------------
			friend class DragHandler;
			class DragHandler
			{
			private:
				// widget dragging trackers
				PositionF m_beginMousePosition;
				PositionF m_beginMovePosition;
				bool m_isMoving = false;

			public:
				void Begin(const PositionF& position, Widget* widget)
				{
					// if not movable, bail out
					if (widget->m_moveBehavior == MoveBehavior::None) return;

					// remember this mouse position. this will be the pivot position as this widget gets dragged around by mouse
					m_beginMousePosition = position;

					// remember the widget's position now. this will be the reference position as it gets dragged around by mouse
					m_beginMovePosition = widget->GetPosition();

					// this widget is now moving
					m_isMoving = true;

					widget->OnDragBegin(DragEventArgs{ GetBeginPosition(), position });
				}

				void Update(const PositionF& position, Widget* widget)
				{
					if (m_isMoving)
					{
						// calculate the mouse movement delta between its position at start of mouse drag and its position now
						// factor in the move state - free? horizontal? vertical?
						VecF delta =
						{
							// if we can move horizontally, use the mouse position. otherwise, use begin position
							(widget->m_moveBehavior & MoveBehavior::Horizontal) ? position.x - m_beginMousePosition.x : 0.0f,

							// if we can move vertically, use the mouse position. otherwise, use begin position
							(widget->m_moveBehavior & MoveBehavior::Vertical) ? position.y - m_beginMousePosition.y : 0.0f
						};

						// we only drag the widget if new position after this drag is different from current position
						PositionF dragpos = m_beginMovePosition + delta;
						if (dragpos == widget->GetPosition()) return;

						// transform widget's position based on mouse movement delta
						widget->SetPosition(m_beginMovePosition + delta);

						widget->OnDragMove(DragEventArgs{ GetBeginPosition(), position });
					}
				}

				void End(const PositionF& position, Widget* widget)
				{
					m_isMoving = false;

					widget->OnDragEnd(DragEventArgs{ GetBeginPosition(), position });
				}

				bool IsDragging() const
				{
					return m_isMoving;
				}

				PositionF GetBeginPosition() const
				{
					return m_beginMousePosition;
				}
			};
#pragma endregion

			bool UnregisterToSystem();

			bool RegisterToSystem()
			{
				return OnRegisterToSystem();
			}

		protected:
			// is just enum (not class) because it needs logical operations
			enum MoveBehavior
			{
				None = 0,
				Horizontal = 1 << 0,
				Vertical = 1 << 1,
				Free = Horizontal | Vertical,
			};

			enum class HitTestBehavior
			{
				Normal,
				AlwaysPass,
				AlwaysFail
			};

			// tree
			Widget* m_parent = nullptr;
			std::vector<std::unique_ptr<Widget>> m_children;

			// transform
			PositionF m_position;
			SizeF m_size;

			// states
			bool m_visible = true;
			bool m_enabled = true;

			// behavior
			bool m_focusable = true;
			bool m_droppable = false;
			MoveBehavior m_moveBehavior = MoveBehavior::Free;
			HitTestBehavior m_hitTestBehavior = HitTestBehavior::Normal;

			// tooltip support
			std::function<void(Widget&, Widget&)> m_tooltipBuilder;

			// widget dragging tracker
			DragHandler m_dragHandler;

			// --------------------------------------------------------------------------------
			// SYSTEM 
			// --------------------------------------------------------------------------------
			virtual UISystem* GetSystem() const
			{
				if (m_parent)
				{
					return m_parent->GetSystem();
				}

				return nullptr;
			}

			virtual bool OnRegisterToSystem()
			{
				return true;
			}

			virtual bool OnUnregisterToSystem()
			{
				return true;
			}

			// --------------------------------------------------------------------------------
			// CHANGE PARAMETER HANDLERS
			// --------------------------------------------------------------------------------
			virtual void OnPositionChanged(const PositionF& oldPos, const PositionF& newPos)
			{
				// default implementation does nothing. derived class can override this to react to position change
			}

			virtual void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
			{
				// default implementation does nothing. derived class can override this to react to size change
			}

			virtual void OnResourceChange()
			{
				// default implementation does nothing. derived class can override this to react to resource change
			}

			// --------------------------------------------------------------------------------
			// DRAG AND DROP EVENT HANDLERS
			// --------------------------------------------------------------------------------
			virtual void OnDrop(Widget* dragged)
			{

			}

			// --------------------------------------------------------------------------------
			// INPUT HANDLERS
			// --------------------------------------------------------------------------------
			virtual void OnMouseDown(const PositionF& position)
			{

			}

			virtual void OnMouseUp(const PositionF& position)
			{

			}

			virtual void OnMouseMove(const PositionF& position)
			{

			}

			virtual void OnMouseEnter()
			{
			}

			virtual void OnMouseLeave()
			{
			}

			virtual void OnKeyDown(int key)
			{
			}

			virtual void OnKeyUp(int key)
			{
			}

		public:
			struct DragEventArgs
			{
				PositionF beginPosition;
				PositionF currentPosition;

				VecF Delta() const
				{
					return currentPosition - beginPosition;
				}
			};

			virtual ~Widget() = default;

			enum class HorizontalAlignment
			{
				Left,
				Right,
				Center
			};
			enum class VerticalAlignment
			{
				Top,
				Bottom,
				Center
			};

			// --------------------------------------------------------------------------------
			// DRAG AND DROP
			// --------------------------------------------------------------------------------
			void DropAccepted(Widget* widget)
			{
				OnDrop(widget);
			}

			// --------------------------------------------------------------------------------
			// HIERARCHY
			// --------------------------------------------------------------------------------
			void AddChild(std::unique_ptr<Widget> child)
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

			void RemoveChild(Widget* widget)
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

			void RemoveChildren()
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

			Widget* GetParent() const
			{
				return m_parent;
			}

			// remove a widget in this widget tree. this will traverse through this widget's tree to find the widget
			// if found, removes it as well as its tree. returns true if successfully found and removed
			bool Remove(Widget* widget)
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

			// checks if this widget is descendant of given widget
			bool IsDescendantOf(const Widget* ancestor) const
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

			bool HasChildren() const
			{
				return m_children.size() > 0;
			}

			void MoveChildTo(Widget* child, Widget* newParent)
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

				// add to new parent
				newParent->AddChild(std::move(movedChild));
			}

			// --------------------------------------------------------------------------------
			// Z ORDER
			// --------------------------------------------------------------------------------
			void BringChildToFront(Widget* child)
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

			void BringToFront()
			{
				// if this has no parent, then it has no siblings. then it does not have z order
				if (!m_parent) return;

				m_parent->BringChildToFront(this);
			}

			// --------------------------------------------------------------------------------
			// STATE
			// --------------------------------------------------------------------------------
			void Show()
			{
				m_visible = true;
			}

			void Hide()
			{
				m_visible = false;
			}

			bool IsVisible() const
			{
				return m_visible;
			}

			void Enable()
			{
				m_enabled = true;
			}

			void Disable()
			{
				m_enabled = false;
			}

			bool IsEnabled() const
			{
				// if this widget is disabled, can return now
				if (!m_enabled) return false;

				// widgets has dependency on their parents/ascendants when it comes to enable state
				// if parent is disabled, then this must be disabled too.
				if (m_parent) return m_parent->IsEnabled();

				// if this is enabled as well as its ascendants, then this is enabled
				return true;
			}

			// --------------------------------------------------------------------------------
			// BEHAVIOR
			// --------------------------------------------------------------------------------
			bool IsFocusable() const
			{
				return m_focusable;
			}

			bool IsDroppable() const
			{
				return m_droppable;
			}

			// --------------------------------------------------------------------------------
			// TRANSFORM
			// --------------------------------------------------------------------------------
			float GetWidth() const
			{
				return m_size.width;
			}

			float GetHeight() const
			{
				return m_size.height;
			}

			SizeF GetSize() const
			{
				return m_size;
			}

			engine::event::Event<const SizeF&> OnResize;

			void SetSize(const SizeF& size)
			{
				// if size did not change, no need to update and invoke events
				// commenting this out coz there seems to be a bug related to scrolling viewport
				if (m_size == size) return;

				SizeF oldSize = m_size;
				m_size = size;
				OnResize(size);
				OnSizeChanged(oldSize, size);
			}

			PositionF GetAbsolutePosition() const
			{
				PositionF position = m_position;
				if (m_parent)
				{
					position += m_parent->GetAbsolutePosition();
				}
				return position;
			}

			engine::event::Event<const PositionF&> OnMove;
			void SetPosition(const PositionF& pos)
			{
				// if position did not change, no need to update and invoke events
				if (m_position == pos) return;

				PositionF oldPos = m_position;
				m_position = pos;
				OnMove(pos);
				OnPositionChanged(oldPos, m_position);
			}

			PositionF GetPosition() const
			{
				return m_position;
			}

			RectF GetAbsoluteRect() const
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

			// --------------------------------------------------------------------------------
			// HIT TEST
			// --------------------------------------------------------------------------------

			bool Contains(const PositionF& position) const
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

			// --------------------------------------------------------------------------------
			// INPUT
			// --------------------------------------------------------------------------------

			engine::event::Event<const DragEventArgs&> OnDragBegin;
			engine::event::Event<const DragEventArgs&> OnDragMove;
			engine::event::Event<const DragEventArgs&> OnDragEnd;

			void MouseDown(const PositionF& position)
			{
				// let derived widget handle mouse down event first
				OnMouseDown(position);

				m_dragHandler.Begin(position, this);

			}

			void MouseUp(const PositionF& position)
			{
				m_dragHandler.End(position, this);

				// now we handle mouse up event after we set its to state to NOT moving
				OnMouseUp(position);
			}

			void MouseMove(const PositionF& position)
			{
				m_dragHandler.Update(position, this);

				// handle this mouse event after this widget updates its position from mouse move
				OnMouseMove(position);
			}

			void MouseEnter()
			{
				OnMouseEnter();
			}

			void MouseLeave()
			{
				OnMouseLeave();
			}

			void KeyDown(int key)
			{
				OnKeyDown(key);
			}

			void KeyUp(int key)
			{
				OnKeyUp(key);
			}

			// --------------------------------------------------------------------------------
			// FOCUS
			// --------------------------------------------------------------------------------
			virtual void OnGotFocus()
			{
			}

			virtual void OnLostFocus()
			{
			}

			// --------------------------------------------------------------------------------
			// TREE TRAVERSAL
			// --------------------------------------------------------------------------------
			enum SearchFlags
			{
				Visible = 1 << 0,
				Enabled = 1 << 1,
				Focusable = 1 << 2,
			};

			// traverse through the tree and find the top-most widget that intersects with point
			Widget* FindTopWidgetAt(const PositionF& position, unsigned int flag)
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
			Widget* FindTopChildAt(const PositionF& position, int flag)
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

			Widget* FindAndResolveZOrderAt(const PositionF& position, int flag)
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

			template<typename Func>
			void ForEachChild(const Func& func)
			{
				for (const std::unique_ptr<Widget>& child : m_children)
				{
					func(child.get());
				}
			}

			template<typename Func>
			void ForEachChild(const Func& func) const
			{
				for (const std::unique_ptr<Widget>& child : m_children)
				{
					func(child.get());
				}
			}

			template<typename Func>
			bool ForEachWidget(const Func& func)
			{
				if (!func(this)) return false;

				for (const std::unique_ptr<Widget>& child : m_children)
				{
					if (!child->ForEachWidget(func)) return false;
				}

				return true;
			}

			// --------------------------------------------------------------------------------
			// TOOLTIP
			// --------------------------------------------------------------------------------

			bool HasTooltip() const
			{
				return m_tooltipBuilder != nullptr;
			}

			void BuildTooltip(Widget& tooltip)
			{
				if (m_tooltipBuilder)
				{
					m_tooltipBuilder(*this, tooltip);
				}
			}

			void SetTooltip(std::function<void(Widget&, Widget&)> builder)
			{
				m_tooltipBuilder = std::move(builder);
			}

			// --------------------------------------------------------------------------------
			// Draw
			// --------------------------------------------------------------------------------
			virtual void Draw(const UIDrawContext& context) const
			{
				// default implementation does nothing. derived class can override this to draw itself
			}

			// --------------------------------------------------------------------------------
			// RESOURCE
			// --------------------------------------------------------------------------------
			virtual void ResourceChange()
			{
				OnResourceChange();
			}

		};
#pragma endregion

#pragma region // Layer
		class Layer : public Widget
		{
		public:
			enum Type
			{
				Popup,
				Modal,
				Menu,
				SubMenu
			};

		private:
			friend class LayerStack;

			Widget* m_owner;
			UISystem* m_system;
			Type m_type;

		protected:
			UISystem* GetSystem() const override final
			{
				return m_system;
			}

		public:
			struct BuildDescription
			{
				PositionF position = {};
				SizeF size = {};
				std::function<void(Widget*)> builder = nullptr;
				Type type = Type::Popup;
				bool movable = false;
			};

			Layer(UISystem* system, Widget* owner, const PositionF& pos, const SizeF& size, const Type& type, bool movable) :
				m_owner(owner),
				m_system(system),
				m_type(type)
			{
				m_moveBehavior = movable ? Widget::MoveBehavior::Free : Widget::MoveBehavior::None;
				SetPosition(pos);
				SetSize(size);
				m_focusable = false;
			}

			Widget* GetOwner() const
			{
				return m_owner;
			}

			bool IsModal() const
			{
				return m_type == Type::Modal;
			}

			bool IsMenu() const
			{
				return m_type == Type::Menu;
			}

			bool IsPopup() const
			{
				return m_type == Type::Popup;
			}

			Type GetType() const
			{
				return m_type;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawLayer(*this, context);
			}
		};

		// design consideration
		// - enforce a policy where in finding route, search stops once a modal overlay did not intersect with input point
		class LayerStack
		{
		private:
			std::vector<std::unique_ptr<Layer>> m_layers;

		public:
			struct Route
			{
				Widget* overlay = nullptr;
				Widget* target = nullptr;
				int index = -1;
				bool isBlockedByModal = false;
			};

			LayerStack()
			{
			}

			size_t Size() const
			{
				return m_layers.size();
			}

			// Collapses the overlay stack starting at the specified overlay index.
			//
			// ---------------------------------------------------------------------------------
			// DESIGN NOTES
			// ---------------------------------------------------------------------------------
			// Layer collapse is always performed from a overlay downward toward the top
			// of the overlay stack.
			//
			// Example:
			//
			//	Stack:
			//		[Layer A]
			//		[Layer B]
			//		[Layer C]
			//
			//	CollapseAt(B)
			//
			//	Result:
			//		[Layer A]
			//
			// Layer B and all overlays above it are removed.
			//
			// ---------------------------------------------------------------------------------
			// CASCADED OVERLAY COLLAPSE
			// ---------------------------------------------------------------------------------
			//
			// Overlays may contain OverlayTriggers that own child overlays higher in the stack.
			//
			// Example:
			//
			//	Layer A
			//		contains Trigger B
			//
			//	Layer B
			//		contains Trigger C
			//
			//	Layer C
			//
			// During collapse, overlay widgets are first unregistered from the UISystem
			// before the overlay itself is erased from the overlay stack.
			//
			// While unregistering:
			//
			//	OverlayTrigger::OnUnregisterToSystem()
			//		-> UISystem::UnregisterLayer()
			//			-> CollapseByOwner()
			//
			// may recursively request collapse of child overlays higher in the stack.
			//
			// This is safe because:
			//	- overlay ownership is acyclic
			//	- overlay stack destruction only proceeds upward
			//	- overlay mutations only remove suffixes of the overlay stack
			//	- traversal is index-based (not iterator-based)
			//	- the overlay stack is never reordered during collapse
			//
			// Example:
			//
			//	Initial stack:
			//		[A][B][C]
			//
			//	CollapseAt(A)
			//
			//	1. A unregisters Trigger B
			//	2. Trigger B collapses B
			//	3. B unregisters Trigger C
			//	4. Trigger C collapses C
			//
			// Each nested collapse only removes overlays above the current overlay.
			//
			// Because nested collapses only shrink the end of the overlay stack,
			// the outer forward traversal remains valid and will naturally terminate
			// once the overlay stack size becomes smaller than the current traversal index.
			//
			// ---------------------------------------------------------------------------------
			// IMPORTANT INVARIANT
			// ---------------------------------------------------------------------------------
			//
			// This method is only safe because overlay collapse semantics are strictly:
			//
			//	- synchronous
			//	- upward-only
			//	- suffix-removing
			//
			// Future changes such as below may invalidate these assumptions and require a deferred mutation model.
			//	- arbitrary overlay removal
			//	- overlay insertion during collapse
			//	- overlay reordering
			//	- deferred destruction
			//	- async/evented mutation
			void CollapseAt(const Route& result)
			{
				int index = result.index < 0 ? 0 : result.index;

				// index can be out of bounds. if there are no active overlays, and this is called, if index = 0, then this condition is valid
				if (index >= (int)m_layers.size()) return;

				// since we're removing overlays, their children must unregister to system.
				for (size_t i = index; i < m_layers.size(); i++)
				{
					m_layers[i]->RemoveChildren();
					m_layers[i]->OnUnregisterToSystem();
				}

				// after unregistering overlays' tree, remove them 
				m_layers.erase(m_layers.begin() + index, m_layers.end());
			}

			void CollapseAbove(const Route& route)
			{
				Route routeAbove = route;
				routeAbove.index++;
				CollapseAt(routeAbove);
			}

			void Collapse()
			{
				Route route;
				route.index = 0;
				CollapseAt(route);
			}

			// this is the only way to add a new overlay in the stack and it will always end it at the end of the stack
			void Add(std::unique_ptr<Layer> overlay)
			{
				m_layers.push_back(std::move(overlay));
			}

			Route FindRouteByOwner(Widget* owner)
			{
				Route result{ nullptr, nullptr, -1 };

				// check if any active overlay is owned by given owner
				for (int i = 0; i < m_layers.size(); i++)
				{
					// if this widget is an owner of existing overlay, then overlay is active. collapse overlay stack on it
					if (m_layers[i].get()->GetOwner() == owner)
					{
						result.overlay = m_layers[i].get();
						result.target = m_layers[i].get();
						result.index = i;
						break;
					}
				}

				return result;
			}

			// collapses overlay stack on overlay with the specified owner widget
			void CollapseByOwner(Widget* owner)
			{
				// find the active overlay that is owned by given owner, if any
				Route result = FindRouteByOwner(owner);
				if (!result.overlay) return;

				// if found, since you get the index, create Route and set the index. collapse on it
				CollapseAt(result);
			}

			// traverse through the overlay stack from bottom to top
			template<typename Func>
			void ForEach(const Func& func)
			{
				for (std::vector<std::unique_ptr<Layer>>::iterator it = m_layers.begin(); it != m_layers.end(); it++)
				{
					func(it->get());
				}
			}

			Layer& Bottom() const
			{
				if (m_layers.empty())
				{
					throw std::runtime_error("Querying an empty stack is wrong.");
				}

				return *m_layers.front().get();
			}

			Layer& Top() const
			{
				if (m_layers.empty())
				{
					throw std::runtime_error("Querying an empty stack is wrong.");
				}

				return *m_layers.back().get();
			}

			// find which top-most active overlay that intersects with given point
			Route FindRouteFromTopAt(const PositionF& position, int flags)
			{
				Route result;

				for (int i = (int)m_layers.size() - 1; i >= 0; i--)
				{
					Widget* widget = m_layers[i]->FindTopWidgetAt(position, flags);
					if (widget)
					{
						result.target = widget;
						result.index = i;
						result.overlay = m_layers[i].get();
						result.isBlockedByModal = false;
						break;
					}
					// if this overlay did not intersect with point, check if it's modal
					else
					{
						// is this overlay a modal? if yes, stop right here. modal overlays when active is the only widget that can absorb user input
						if (m_layers[i]->IsModal())
						{
							result.index = i;
							result.isBlockedByModal = true;
							break;
						}
					}
				}

				return result;
			}

			bool IsExpanded(const Widget* owner) const
			{
				// check if any active overlay is owned by given owner
				for (int i = 0; i < m_layers.size(); i++)
				{
					// if this widget is an owner of existing overlay, then overlay is active. 
					if (m_layers[i].get()->GetOwner() == owner)
					{
						return true;
					}
				}

				return false;
			}

		};

		class LayerManager
		{
		private:
			// internal data structure to store command request 
			struct Command
			{
				enum Type
				{
					Add,
					Remove,
					Collapse,
				};

				Type command;
				Widget* owner = nullptr;
				int index;
				PositionF position;
				SizeF size;
				std::function<void(Widget*)> builder;
				Layer::Type type;
				bool movable;
			};

			LayerStack m_stack;
			UISystem* m_system;
			Dictionary<Widget*, Layer::BuildDescription> m_buildDescriptions;
			std::vector<Command> m_commands;

		public:
			LayerManager(UISystem* system) :
				m_system(system)
			{
			}

			void CollapseAbove(const LayerStack::Route& route)
			{
				m_stack.CollapseAbove(route);
			}

			// finds the top-most layer that intersects with given position and valid with given flags
			LayerStack::Route FindRouteFromTopAt(const PositionF& position, int flags)
			{
				return m_stack.FindRouteFromTopAt(position, flags);
			}

			void Collapse()
			{
				m_stack.Collapse();
			}

			void FlushCommands()
			{
				m_commands.clear();
			}

			// given a overlay stack route result, let overlay tree handle mouse down by performing overlay stack collapse if needed, 
			// and process on queue overlay command requests e.g. toggle up/down a overlay
			void ProcessCommandRequests()
			{
				// handle overlay add/remove queue requests
				for (Command& cmd : m_commands)
				{
					switch (cmd.command)
					{
						// remove/toggle off the overlay that is owned by widget from overlay request
					case Command::Remove:
					{
						// we already have the index of the overlay stack that we want to collapsed at. just validate and collapse with it
						if (cmd.index >= 0 && cmd.index < m_stack.Size())
						{
							LayerStack::Route route{};
							route.index = cmd.index;
							m_stack.CollapseAt(route);
						}
						break;
					}
					// add this overlay on top of stack
					case Command::Add:
					{
						// create the overlay
						std::unique_ptr<Layer> overlay = std::make_unique<Layer>(m_system, cmd.owner, cmd.position, cmd.size, cmd.type, cmd.movable);

						// if it has a payload, build it and add to overlay as child
						if (cmd.builder)
						{
							cmd.builder(overlay.get());
						}

						// finally, add overlay to top of stack
						m_stack.Add(std::move(overlay));
						break;
					}
					case Command::Collapse:
					{
						m_stack.Collapse();
						break;
					}
					default:
						break;
					}
				}

				// flush the commands after consuming them
				m_commands.clear();
			}

			// toggle the overlay
			void QueueToggle(Widget* owner)
			{
				// check if there is an active overlay that is owned by given owner
				LayerStack::Route result = m_stack.FindRouteByOwner(owner);

				// if the owner's overlay is already active, queue it for removal/collapse
				if (result.overlay)
				{
					Command cmd{};
					cmd.command = Command::Remove;
					cmd.index = result.index;
					cmd.owner = owner;
					m_commands.push_back(cmd);
					return;
				}

				// this widget's overlay does not exist in overlay stack. create it and add into top of the stack. but first, check if this widget has registered overlay build command
				if (!m_buildDescriptions.Has(owner))
				{
					throw std::runtime_error("command for this owner does not exist");
				}

				// get the popu build command 
				Layer::BuildDescription& desc = m_buildDescriptions.Get(owner);

				// create overlay build request
				Command cmd{};
				cmd.command = Command::Add;
				cmd.owner = owner;
				cmd.position = owner->GetAbsolutePosition() + desc.position;
				cmd.size = desc.size;
				cmd.builder = desc.builder;
				cmd.type = desc.type;
				cmd.movable = desc.movable;
				m_commands.push_back(cmd);
			}

			// register a overlay build description owned by given widget
			bool Register(Widget* widget, const Layer::BuildDescription& desc)
			{
				return m_buildDescriptions.Register(widget, desc);
			}

			// unregister a overlay build description owned by given widget
			bool Unregister(Widget* owner)
			{
				// collapse overlay stack at the overlay of this owner, if any
				m_stack.CollapseByOwner(owner);

				// then we unregister it from our overlay layer
				return m_buildDescriptions.Unregister(owner);
			}

			// traverse through the overlay stack from bottom to top
			template<typename Func>
			void ForEach(const Func& func)
			{
				m_stack.ForEach(func);
			}

			// queue add overlay based on build description as this has no owner
			void QueueAdd(const Layer::BuildDescription& desc)
			{
				// create overlay build command on top of stack based on build description
				Command cmd{};
				cmd.command = Command::Add;
				cmd.owner = nullptr;
				cmd.position = desc.position;
				cmd.size = desc.size;
				cmd.builder = desc.builder;
				cmd.type = desc.type;
				cmd.movable = desc.movable;
				m_commands.push_back(cmd);
			}

			void QueueCollapse(int index = 0)
			{
				Command cmd{};
				cmd.command = Command::Remove;
				cmd.index = index;
				m_commands.push_back(cmd);
			}

			Layer& Bottom() const
			{
				if (!m_stack.Size())
				{
					throw std::runtime_error("stack is empty. querying for first is wrong");
				}

				return m_stack.Bottom();
			}

			bool IsExpanded(const Widget* owner) const
			{
				return m_stack.IsExpanded(owner);
			}
		};

#pragma endregion

#pragma region // UIRenderer
		class UIRenderer
		{
		private:
		public:
			static void Draw(const UIDrawContext& context, const Widget& widget)
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
		};
#pragma endregion

#pragma region // Tooltip
		class Tooltip : public Widget
		{
		private:
		public:
			Tooltip()
			{
				m_moveBehavior = MoveBehavior::None;
				m_focusable = false;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawTooltip(*this, context);
			}
		};

		// design consideration:
		// - only one tooltip exists
		// - tooltip information belongs to a widget
		// - not all widgets has tooltip
		// - tooltip should not show during capture/drag
		// - tooltip position is relative to owner widget
		// - delayed appearance is a system timing concern, not widget concern
		// design consideration:
		// - "it does not decide whether to show or hide a tooltip, but if it's told to show a tooltip, it will show it his way"
		//		- what it means is that UISystem decides if a tooltip is shown or hidden and requests ToolTipManager to do it
		//		- but ToolTipManager decides how tooltip is shown e.g. TooltipManager will apply delay before showing tooltip
		// - the following policies are enforced by UISystem regarding showing or hiding of tooltip
		//		- if mouse is captured by widget, tooltip must be hidden
		//		- if mouse hovers over a widget while mouse is not captured and widget has tooltip, tooltip is displayed
		class TooltipManager
		{
		private:
			Tooltip m_tooltip;
			Widget* m_owner;

		public:
			TooltipManager() :
				m_owner(nullptr)
			{
				m_tooltip.SetPosition({ 0,0 });
				m_tooltip.SetSize({ 0,0 });
			}

			void Hide()
			{
				m_tooltip.RemoveChildren();
				m_tooltip.SetPosition({ 0,0 });
				m_tooltip.SetSize({ 0,0 });
				m_tooltip.Hide();
				m_owner = nullptr;
			}

			// toggle the overlay
			void Show(Widget* hover)
			{
				if (!hover || !hover->HasTooltip())
				{
					Hide();
					return;
				}

				if (hover != m_owner)
				{
					// do this first to flush the old tooltip
					Hide();

					// rebuild tooltip for new owner
					hover->BuildTooltip(m_tooltip);

					// this is new tooltip owner now
					m_owner = hover;
				}

				// show tooltip
				m_tooltip.Show();
			}

			const Widget* Get() const
			{
				return &m_tooltip;
			}
		};
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
			UISystem* m_system;
			Dictionary<Widget*, DragDropContext> m_draggables;

		protected:
			UISystem* GetSystem() const override final
			{
				return m_system;
			}

		public:
			DragDropLayer(UISystem* system) :
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
			void Begin(Widget* draggable)
			{
				// only reason why our draggables contain something is if we previously started dragging a draggable and has not dropped it yet.
				// starting another drag while in this state is unacceptable. i should not happen
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

			void End(Widget* draggable, Widget* newParent)
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
					context.originalParent :		// new parent is not droppable, so using the original parent
					context.originalParent;		// new parent is invalid, so using the original parent 

				// check what will be the position of the drag widget once it is dropped - is it back to original position or now in the new parent?
				// NOTE: we calculate position here before moving child to new parent because we refer to drag widget's absolute position here prior to being moved to new parent
				PositionF pos = newParent ?												// is new parent valid?
					newParent->IsDroppable() ?											// is new parent droppable?
					draggable->GetAbsolutePosition() - parent->GetAbsolutePosition() :	// new parent is valid, so position is now relative to new parent
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

				// TODO: for now, let's just always call this when drop happens. we don't know what use cases are for handling this yet. let's deal with it once we hit them use cases
				// let parent invoke drop acceptance event
				parent->DropAccepted(draggable);

				// clear our draggables list
				m_draggables.Clear();
			}
		};
#pragma endregion

#pragma region // UIResources
		struct UIResources
		{
			IFontAtlas* defaultFont = nullptr;
			IFontAtlas* highlightFont = nullptr;
			IFontAtlas* titleFont = nullptr;

			enum class FontType
			{
				Default,
				Highlight,
				Title
			};
		};
#pragma endregion

#pragma region // UISystem
		class UISystem
		{
		private:
			LayerManager m_layerManager;
			TooltipManager m_tooltipManager;
			DragDropLayer	m_DragDropLayer;

			Widget* m_mouseCapture = nullptr;
			Widget* m_mouseOver = nullptr;
			Widget* m_focus = nullptr;

			UIResources m_resources;

			void SetFocus(Widget* widget)
			{
				// if we're setting the same widget that is already in focus, do nothing
				if (m_focus == widget) return;

				// since we're changing focus, notify current focus it's about to lose focus
				if (m_focus)
				{
					m_focus->OnLostFocus();
					m_focus = nullptr;
				}

				// if new focus widget does not exist, bail out
				if (!widget) return;

				// if this widget is not focusable, bail out
				if (!widget->IsFocusable()) return;

				// this new widget is valid to be new focus, notify it
				m_focus = widget;
				if (m_focus)
				{
					m_focus->OnGotFocus();
				}
			}

			void SetCapture(Widget* widget)
			{
				m_mouseCapture = widget;
			}

			Widget& Root() const
			{
				return m_layerManager.Bottom();
			}

		public:
			void SetFont(IFontAtlas* font, UIResources::FontType type)
			{
				bool fontChanged = false;
				switch (type)
				{
				case UIResources::FontType::Default:
					if (m_resources.defaultFont != font) fontChanged = true;
					m_resources.defaultFont = font;
					break;
				case UIResources::FontType::Highlight:
					if (m_resources.highlightFont != font) fontChanged = true;
					m_resources.highlightFont = font;
					break;
				case UIResources::FontType::Title:
					if (m_resources.titleFont != font) fontChanged = true;
					m_resources.titleFont = font;
					break;
				default:
					break;
				}

				// update all widgets if font changed as they may need to recalculate their layout based on new font
				if (fontChanged)
				{
					m_layerManager.ForEach([](Widget* widget)
						{
							widget->ForEachWidget([](Widget* widget)
								{
									widget->ResourceChange();
									return true;
								});
						});
				}
			}

			IFontAtlas* GetFont(UIResources::FontType type) const
			{
				switch (type)
				{
				case UIResources::FontType::Default:
					return m_resources.defaultFont;
				case UIResources::FontType::Highlight:
					return m_resources.highlightFont;
				case UIResources::FontType::Title:
					return m_resources.titleFont;
				default:
					return nullptr;
				}
			}

			UISystem() :
				//m_layoutTree(this),
				m_layerManager(this),
				m_DragDropLayer(this)
			{
				// define build for root layer and queue on layer manager
				Layer::BuildDescription root
				{
					PositionF{0,0},
					SizeF{0, 0},
					nullptr,
					Layer::Modal,
					false
				};
				m_layerManager.QueueAdd(root);

				// build the root layer
				m_layerManager.ProcessCommandRequests();
			}

			void SetSize(const SizeF& size)
			{
				Root().SetSize(size);
			}

			void SetPosition(const PositionF& pos)
			{
				Root().SetPosition(pos);
			}

			void Show()
			{
				Root().Show();
			}

			void Draw(UIDrawContext& context)
			{
				// set the input state in context so that widgets can use it when drawing themselves
				context.capture = m_mouseCapture;
				context.hover = m_mouseOver;
				context.focus = m_focus;

				// draw overlays
				m_layerManager.ForEach([&](Widget* widget)
					{
						UIRenderer::Draw(context, *widget);
					});

				// draw tooltip
				UIRenderer::Draw(context, *m_tooltipManager.Get());

				// draw draggable
				m_DragDropLayer.ForEachChild([&](Widget* widget)
					{
						UIRenderer::Draw(context, *widget);
					});
			}

			// this "detaches" the widget from system. if widget is mouse capture, hover, or focus, these states will be reset to null
			void Detach(Widget* widget)
			{
				if (m_mouseCapture == widget) SetCapture(nullptr);
				if (m_focus == widget) SetFocus(nullptr);
				if (m_mouseOver == widget) m_mouseOver = nullptr;
			}

			// scenario 1 - no overlay exists, overlay trigger is clicked
			//		- system does not check overlay tree for hit, as it is empty
			//		- overlay trigger requests system to toggle its overlay
			//		- system does not have its overlay yet so queue it to add
			//		- system does not remove any overlay in tree. does nothing
			//		- system handles all queued overlay requests
			// 
			// scenario 2 - overlays exists, overlay trigger is clicked, and its overlay already exists
			//		- none of the overlays in overlay tree is hit, so all is queued for removal
			//		- overlay trigger requests system to toggle its overlay
			//		- system have its overlay so queue it to remove
			//		- system removes all existing overlay in overlay tree
			//		- system handles all queued overlay requests
			// 
			// scenario 3 - overlays exists, overlay trigger is clicked
			// 		- none of the overlays in overlay tree is hit, so all is queued for removal
			//		- overlay trigger requests system to toggle its overlay
			//		- system does not have its overlay yet so queue it to add
			//		- system removes all existing overlay in overlay tree
			//		- system handles all queued overlay requests
			// 
			// scenario 4 - overlays exists, overlay trigger's overlay is active, mouse clicked somewhere not in any overlay nor in overlay trigger
			// 		- none of the overlays in overlay tree is hit, so all is queued for removal
			//		- overlay trigger does nothing. it did not get hit.
			//		- system removes all existing overlay in overlay tree
			//		- system has no pop requests to handle, does nothing
			// 
			// scenario 5 - overlay exists, overlay trigger's overlay is active, mouse clicked in one of the existing overlays
			//		- system finds overlay that got hit in stack. queue overlays above it for removal
			//		- overlay trigger does nothing. it did not get hit.
			//		- system removes all overlays on queue for removal
			//		- system has no pop requests to handle, does nothing
			// 
			// scenario 6 - no overlay exists, mouse clicked somewhere not in any overlay nor in overlay trigger
			//		- system does not check overlay tree for hit, as it is empty
			//		- overlay trigger does nothing. it did not get hit.
			//		- system does not remove any overlay in tree. does nothing
			//		- system has no pop requests to handle, does nothing
			// 
			// scenario 7 - overlay exists, overlay trigger a's overlay is active, but overlay trigger b is clicked
			// 		- none of the overlays in overlay tree is hit, so all is queued for removal
			//		- overlay trigger b requests system to toggle its overlay
			//		- system checks for popbutton b's overlay. if it exists, queue it for removal. otherwise, queue it for add
			//		- overlay trigger a does nothing. it did not get hit
			//		- system removes all existing overlay in overlay tree
			//		- system handles all queued overlay requests
			// 
			// scenario 8 - overlay opens a child overlay. this only happens if overlay contains a overlay trigger as child (only overlay trigger can request to spawn a overlay, as of now)
			//		- system finds overlay that got hit in stack. queue overlays above it for removal
			//		- overlay trigger clicked requests system to toggle its overlay
			//		- system checks for popbutton's overlay. if it exists, queue it for removal. otherwise, queue it for add
			//		- system removes all overlays on queue for removal
			//		- system handles all queued overlay requests
			// 
			// scenario 9 - modal overlay exists
			//		- THIS IS PROBLEM FOR ANOTHER DAY. WE DON'T HAVE MODAL YET
			//
			void MouseDown(const PositionF& p)
			{
				m_layerManager.FlushCommands();

				// find top overlay that intersects with point. overlay must be visible and enabled
				LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

				// check if result says we're block by modal. this means that a modal layer exist and did not intersect with point and this blocks search to succeeding layer stack
				if (result.isBlockedByModal)
				{
					// if block by modal, collapse above it. we should not collapse modals. it should only be collapsed via command
					m_layerManager.CollapseAbove(result);

					// in case focus, hover and capture are set to widgets that belong to overlay that collapsed, they are reset safely via UnregisterToSystem>Detach
					return;
				}

				// if we reach this point, we should be able to find the top widget that intersects with point. simultaneously we can resolve Z order as we traverse to find the top widget
				// since bottom layer is a modal (root), it should always exist therefore we should always expect a valid layer at this point
				// if not, then we must throw exception as this should not happen
				if (!result.overlay)
				{
					throw std::runtime_error("impossible not to find an overlay. why is this so???");
				}

				Widget* widget = result.overlay->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

				// at this point, we should have the top-most widget and Z order is resolved. it's impossible to not find top-most widget, we already have the layer.
				if (!widget)
				{
					throw std::runtime_error("why no top-most widget when we already found the layer??");
				}

				// now we are ready to execute MouseDown event on the clicked widget, if there is one. by right there should be one by this time. 
				widget->MouseDown(p);

				// collapse the overlay stack above the clicked overlay. we do this because:
				// - if none of the overlays were clicked, all active overlay stacks will be collapsed 
				// - if a overlay is clicked, all active overlays on top of it will be collapsed
				m_layerManager.CollapseAbove(result);

				// set capture
				SetCapture(widget);

				// set focus
				SetFocus(widget);

				// hide tooltip. if mouse is down, tooltip should be hidden regardless of where the mouse is clicked
				m_tooltipManager.Hide();
			}

			void MouseUp(const PositionF& p)
			{
				if (!m_mouseCapture) return;
				m_mouseCapture->MouseUp(p);
				m_mouseCapture = nullptr;

				// by right, tooltip of the widget (if it has tooltip) the mouse hovers now should appear... 
				// but after mouse up, we don't have mouse over widget yet, so we don't bother showing tooltip now
			}

			void MouseMove(const PositionF& p)
			{
				// prioritize captured widget to handle mouse move 
				if (m_mouseCapture)
				{
					m_mouseCapture->MouseMove(p);

					// since mouse is captured, tooltip should be hidden
					m_tooltipManager.Hide();

					return;
				}

				// check first if mouse hovers over a overlay
				LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

				// if mouse hovers outside of the top overlay in the stack and down to top-most modal overlay, the route result will be "blocked by modal"
				// this is because when one or more modal overlay exists, the top-most modal overlay and succeeding overlays on top of it are the only ones 
				// allowed to receive mouse event or user input in general. if mouse cursor did not hover over any of them overlays, then mouse move is ignored. 
				if (result.isBlockedByModal)
				{
					// just in case there is a mouse over widget somewhere, let's handle its mouse leave
					if (m_mouseOver)
					{
						m_mouseOver->MouseLeave();
						m_mouseOver = nullptr;
					}

					// make sure to hide any active tooltip as well
					m_tooltipManager.Hide();

					return;
				}

				// if there is no overlay found yet we were not blocked by modal, something is wrong. this cannot happen
				if (!result.overlay)
				{
					throw std::runtime_error("impossible not to find an overlay. why is this so???");
				}

				// find the top widget in this layer's tree that is hovered. we also include disabled widgets in hover check.
				// reason is so that even disable widgets can still have tooltip shown if they have it
				Widget* hover = result.overlay->FindTopWidgetAt(p, Widget::SearchFlags::Visible);

				// let's resolve which widget is mouse over now, if any
				if (hover != m_mouseOver)
				{
					// invoke mouse leave on current mouse hover widget
					if (m_mouseOver)
					{
						m_mouseOver->MouseLeave();
					}

					// just in case we hover outside of root, assuming root is not desktop, hover will be nullptr
					m_mouseOver = hover;
					if (m_mouseOver)
					{
						m_mouseOver->MouseEnter();
					}
				}

				// finally if there is a mouse over widget, let it handle mouse move event
				if (m_mouseOver)
				{
					m_mouseOver->MouseMove(p);
				}

				// if you reach this point, then mouse hovers a widget that might have a tooltip. show it.
				m_tooltipManager.Show(m_mouseOver);
			}

			void KeyDown(int key)
			{
				if (m_focus)
				{
					m_focus->KeyDown(key);
				}
			}

			void KeyUp(int key)
			{
				if (m_focus)
				{
					m_focus->KeyUp(key);
				}
			}

			bool RegisterLayer(Widget* widget, const Layer::BuildDescription& desc)
			{
				return m_layerManager.Register(widget, desc);
			}

			bool UnregisterLayer(Widget* owner)
			{
				return m_layerManager.Unregister(owner);
			}

			void ToggleLayer(Widget* owner)
			{
				m_layerManager.QueueToggle(owner);
			}

			void AddWidget(std::unique_ptr<Widget> widget)
			{
				Root().AddChild(std::move(widget));
			}

			void RemoveWidget(Widget* widget)
			{
				// bail out if invalid
				if (!widget) return;

				// we can now remove this widget. this will remove the widget's whole tree. 
				//if (!m_layoutTree.Remove(widget))
				if (!Root().Remove(widget))
				{
					// let's be strict for now to catch any silent error
					throw std::runtime_error("failed to remove a widget from root");
				}
			}

			void Collapse()
			{
				m_layerManager.QueueCollapse(1);
			}

			void AddLayer(const Layer::BuildDescription& desc)
			{
				m_layerManager.QueueAdd(desc);
			}

			bool IsLayerExpanded(const Widget* owner) const
			{
				return m_layerManager.IsExpanded(owner);
			}

			void Begin()
			{
				m_layerManager.FlushCommands();
			}

			void End()
			{
				// if a overlay trigger is clicked, it might have requested to toggle its overlay. process those requests here
				m_layerManager.ProcessCommandRequests();
			}

			void BeginDrag(Widget* source)
			{
				// for now we just end drag immediately. we can implement this later when we have drag drop scenario
				// but we want to have this method here as placeholder to show where drag drop manager will be used in
				m_DragDropLayer.Begin(source);
			}

			void EndDrag(Widget* draggable, const PositionF& p)
			{
				// 1. find the top-most widget that intersects with given point
				LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

				// if route result is blocked by modal, it means we intersect outside of existing modal layer and there are no other widgets that can be found to drop current dragged widget
				// but if not modal, we must have found the layer that intersects with  point
				Widget* target = nullptr;
				if (!result.isBlockedByModal)
				{
					// but check first if layer is really valid. it must.
					// if there is no overlay found yet we were not blocked by modal, something is wrong. this cannot happen
					if (!result.overlay)
					{
						throw std::runtime_error("impossible not to find an overlay. why is this so???");
					}

					// let's now find the top widget in this layer's tree that intersects with the point
					target = result.overlay->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);
				}


				// 2. pass that widget to dragdrop layer so it will attemp to drop the widget being drag into it
				m_DragDropLayer.End(draggable, target);
			}
		};

		bool Widget::UnregisterToSystem()
		{
			OnUnregisterToSystem();

			UISystem* system = GetSystem();
			if (system) system->Detach(this);
			return true;
		}
#pragma endregion

#pragma region // OverlayTrigger
		class OverlayTrigger : public Widget
		{
		protected:
			Layer::BuildDescription m_buildDesc;

			// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
			bool OnRegisterToSystem() override final
			{
				UISystem* system = GetSystem();
				if (system)
				{
					// be strict for now
					if (!system->RegisterLayer(this, m_buildDesc))
					{
						throw std::runtime_error("failed to register layer");
					}
				}
				return true;
			}

			// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
			bool OnUnregisterToSystem() override final
			{
				UISystem* system = GetSystem();
				if (system)
				{
					// be strict for now
					if (!system->UnregisterLayer(this))
					{
						throw std::runtime_error("failed to unregister layer");
					}
				}

				return true;
			}

			// requests system to toggle this widget's overlay
			void Toggle()
			{
				UISystem* system = GetSystem();
				if (system)
				{
					system->ToggleLayer(this);
				}
			}

			void OnMouseDown(const PositionF& position) override final
			{
				Toggle();
			}

		public:
			OverlayTrigger(const Layer::BuildDescription& buildDesc) :
				m_buildDesc(buildDesc)
			{
				m_moveBehavior = MoveBehavior::None;
			}

			virtual bool HasTooltip() const
			{
				return true;
			}

			virtual void BuildTooltip(Widget& tooltip)
			{
				// this is just for debug purposes. can formalize this later
				tooltip.SetSize({ 80,30 });
				tooltip.SetPosition(GetAbsolutePosition() + PositionF{ GetSize().width + 5, 0 });
			}
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
			virtual void OnMouseDown(const PositionF& position)
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
			virtual void OnMouseUp(const PositionF& position)
			{
				UISystem* system = GetSystem();
				if (!system)
				{
					throw std::runtime_error("widget is not attached to any UISystem");
				}

				// let system know we want this widget to drop
				system->EndDrag(this, position);
			}

		public:
			Draggable()
			{
				m_moveBehavior = Widget::MoveBehavior::Free;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawDraggable(*this, context);
			}
		};
#pragma endregion

#pragma region // BoundRef

		template<typename T>
		class BoundRef
		{
		private:
			T m_internal{};   // fallback storage
			T* m_ptr = nullptr; // external binding

			std::function<T()> m_getter;
			std::function<void(const T&)> m_setter;

		public:
			void Bind(T& external)
			{
				m_ptr = &external;
			}

			void Bind(
				std::function<T()> getter,
				std::function<void(const T&)> setter
			)
			{
				m_ptr = nullptr;
				m_getter = std::move(getter);
				m_setter = std::move(setter);
			}

			void Unbind()
			{
				m_ptr = nullptr;
			}

			T Get() const
			{
				if (m_ptr) return *m_ptr;

				if (m_getter) return m_getter();

				return m_internal;
			}

			void Set(const T& value)
			{
				if (m_ptr)
				{
					*m_ptr = value;
					return;
				}

				if (m_setter)
				{
					m_setter(value);
					return;
				}

				m_internal = value;
			}

			bool IsBound() const { return m_ptr != nullptr; }

			void operator = (const T& v)
			{
				Set(v);
			}

			operator T() const
			{
				return Get();
			}

			BoundRef& operator = (const BoundRef& other)
			{
				Set(other.Get());
				return *this;
			}

		};
#pragma endregion

#pragma region // gui controls
		class Image : public Widget
		{
		private:
			std::unique_ptr<IRenderable> m_image;
			Widget::VerticalAlignment m_vAlign;
			Widget::HorizontalAlignment m_hAlign;
			PositionF m_imagePosition;
			bool m_stretch = false;

		protected:
			// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
			bool OnRegisterToSystem() override final
			{
				RefreshLayout();
				return true;
			}

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final
			{
				// refresh cached information about text with new font type
				RefreshLayout();
			}

			bool RefreshLayout()
			{
				if (!m_image)
				{
					m_imagePosition = {};
					return false;
				}

				if (m_stretch)
				{
					m_imagePosition = { 0,0 };
				}
				else
				{
					switch (m_vAlign)
					{
					case Widget::VerticalAlignment::Center:
						m_imagePosition.y = (GetSize().height - m_image->GetSprite().GetHeight()) / 2.0f;
						break;
					case Widget::VerticalAlignment::Top:
						m_imagePosition.y = 0;
						break;
					case Widget::VerticalAlignment::Bottom:
						m_imagePosition.y = GetSize().height - m_image->GetSprite().GetHeight();
						break;
					default:
						break;
					}

					switch (m_hAlign)
					{
					case Widget::HorizontalAlignment::Center:
						m_imagePosition.x = (GetSize().width - m_image->GetSprite().GetWidth()) / 2.0f;
						break;
					case Widget::HorizontalAlignment::Left:
						m_imagePosition.x = 0;
						break;
					case Widget::HorizontalAlignment::Right:
						m_imagePosition.x = GetSize().width - m_image->GetSprite().GetWidth();
						break;
					default:
						break;
					}
				}

				return true;
			}

		public:
			Image(std::unique_ptr<IRenderable> renderable) :
				m_image(std::move(renderable)),
				m_vAlign(Widget::VerticalAlignment::Center),
				m_hAlign(Widget::HorizontalAlignment::Center),
				m_imagePosition({ 0,0 })
			{
				m_moveBehavior = MoveBehavior::None;
				RefreshLayout();
				m_hitTestBehavior = HitTestBehavior::AlwaysFail;
			}

			void EnableStretch(bool stretch)
			{
				m_stretch = stretch;
				RefreshLayout();
			}

			bool IsStretched() const
			{
				return m_stretch;
			}

			PositionF GetImageAbsolutePosition() const
			{
				return GetAbsolutePosition() + m_imagePosition;
			}

			Sprite Get() const
			{
				return m_image->GetSprite();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawImage(*this, context);
			}

			void SetAlignment(Widget::VerticalAlignment vAlign, Widget::HorizontalAlignment hAlign)
			{
				m_vAlign = vAlign;
				m_hAlign = hAlign;
				RefreshLayout();
			}
		};

		class Label : public Widget
		{
		private:
			std::string m_text;
			UIResources::FontType m_fontType;
			Widget::VerticalAlignment m_vAlign;
			Widget::HorizontalAlignment m_hAlign;
			SizeF m_textSize;
			PositionF m_textPosition;

		protected:
			// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
			bool OnRegisterToSystem() override final
			{
				// refresh cached information about text with new font type
				return RefreshLayout();
			}

			// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
			bool OnUnregisterToSystem() override final
			{
				// refresh cached information about text with new font type
				return RefreshLayout();
			}

			void OnResourceChange() override final
			{
				// refresh cached information about text with new font type
				RefreshLayout();
			}

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final
			{
				// refresh cached information about text with new font type
				RefreshLayout();
			}

			bool RefreshLayout()
			{
				UISystem* system = GetSystem();
				if (!system)
				{
					m_textSize = {};
					m_textPosition = {};
					return false;
				}

				IFontAtlas* font = system->GetFont(m_fontType);
				if (!font)
				{
					m_textSize = {};
					m_textPosition = {};
					return false;
				}

				m_textSize = font->GetSize(m_text);

				switch (m_vAlign)
				{
				case Widget::VerticalAlignment::Center:
					m_textPosition.y = (GetSize().height - m_textSize.height) / 2.0f;
					break;
				case Widget::VerticalAlignment::Top:
					m_textPosition.y = 0;
					break;
				case Widget::VerticalAlignment::Bottom:
					m_textPosition.y = GetSize().height - m_textSize.height;
					break;
				default:
					break;
				}

				switch (m_hAlign)
				{
				case Widget::HorizontalAlignment::Center:
					m_textPosition.x = (GetSize().width - m_textSize.width) / 2.0f;
					break;
				case Widget::HorizontalAlignment::Left:
					m_textPosition.x = 0;
					break;
				case Widget::HorizontalAlignment::Right:
					m_textPosition.x = GetSize().width - m_textSize.width;
					break;
				default:
					break;
				}

				return true;
			}

		public:
			Label(const std::string& text, UIResources::FontType fontType = UIResources::FontType::Default) :
				m_text(text),
				m_fontType(fontType),
				m_vAlign(Widget::VerticalAlignment::Center),
				m_hAlign(Widget::HorizontalAlignment::Center),
				m_textSize({ 0,0 }),
				m_textPosition({ 0,0 })
			{
				m_moveBehavior = MoveBehavior::None;
				m_hitTestBehavior = HitTestBehavior::AlwaysFail;
			}

			PositionF GetTextAbsolutePosition() const
			{
				return GetAbsolutePosition() + m_textPosition;
			}

			void SetFontType(UIResources::FontType type)
			{
				m_fontType = type;

				// refresh cached information about text with new font type
				RefreshLayout();
			}

			UIResources::FontType GetFontType() const
			{
				return m_fontType;
			}

			std::string Get() const
			{
				return m_text;
			}

			void Set(const std::string& text)
			{
				m_text = text;

				RefreshLayout();
			}

			void SetAlignment(Widget::VerticalAlignment vAlign, Widget::HorizontalAlignment hAlign)
			{
				m_vAlign = vAlign;
				m_hAlign = hAlign;
				RefreshLayout();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawLabel(*this, context);
			}
		};

		class Frame : public Widget
		{
		private:

		public:
			Frame(bool movable = true, bool droppable = true)
			{
				m_moveBehavior = movable ? MoveBehavior::Free : MoveBehavior::None;
				m_droppable = droppable;
				m_focusable = false;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawFrame(*this, context);
			}
		};

		class Button : public Widget
		{
		private:
		public:
			Button()
			{
				m_moveBehavior = MoveBehavior::None;
			}

			event::Event<> Click;

			void OnMouseUp(const PositionF& position) override
			{
				// did the mouse release occur over this button? if not, then this mouse up is not for us. ignore
				if (!Contains(position)) return;

				// handle click event
				Click();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawButton(*this, context);
			}
		};

		class MenuButton : public Button
		{
		protected:
			Layer::BuildDescription m_buildDesc;

			// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
			bool OnRegisterToSystem() override final
			{
				UISystem* system = GetSystem();
				if (system)
				{
					// be strict for now
					if (!system->RegisterLayer(this, m_buildDesc))
					{
						throw std::runtime_error("failed to register layer");
					}
				}
				return true;
			}

			// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
			bool OnUnregisterToSystem() override final
			{
				UISystem* system = GetSystem();
				if (system)
				{
					// be strict for now
					if (!system->UnregisterLayer(this))
					{
						throw std::runtime_error("failed to unregister layer");
					}
				}

				return true;
			}

			// requests system to toggle this widget's overlay
			void Toggle()
			{
				UISystem* system = GetSystem();
				if (system)
				{
					system->ToggleLayer(this);
				}
			}

			void OnMouseDown(const PositionF& position) override final
			{
				Toggle();
			}

		public:
			MenuButton(const Layer::BuildDescription& buildDesc) :
				m_buildDesc(buildDesc)
			{
				m_moveBehavior = MoveBehavior::None;
				m_buildDesc.type = Layer::Menu;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawMenuButton(*this, context);
			}
		};

		class SubMenuButton : public MenuButton
		{
		protected:
		public:
			SubMenuButton(const Layer::BuildDescription& buildDesc) :
				MenuButton(buildDesc)
			{
				m_buildDesc.type = Layer::SubMenu;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawSubMenuButton(*this, context);
			}
		};

		class MenuItem : public Button
		{
		public:
			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawMenuItem(*this, context);
			}
		};

		class Thumb : public Widget
		{
		private:
		public:
			Thumb()
			{
				m_moveBehavior = MoveBehavior::None;
				m_hitTestBehavior = HitTestBehavior::AlwaysFail; // non interactive
			}

			Thumb(bool isHorizontal)
			{
				m_moveBehavior = isHorizontal ? MoveBehavior::Horizontal : MoveBehavior::Vertical;
				m_hitTestBehavior = HitTestBehavior::AlwaysFail; // non interactive
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawThumb(*this, context);
			}
		};

		class Slider : public Widget
		{
		private:
			float m_min;
			float m_max;
			float m_value;

			bool m_horizontal;
			Widget* m_thumb;
			float m_thumbLength;
			bool m_isDragging;
			int m_steps;

		protected:
			virtual void OnMouseDown(const PositionF& position)
			{
				m_isDragging = true;
				UpdateValueFromPosition(position);
			}

			void OnMouseUp(const PositionF& position) override
			{
				m_isDragging = false;
			}

			void OnMouseMove(const PositionF& position) override
			{
				if (m_isDragging)
				{
					UpdateValueFromPosition(position);
				}
			}

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				UpdateThumbSize();

				UpdateThumbPosition();
			}

			void UpdateThumbPosition()
			{
				// value is in range between min and max. normalize it. if max < min, set normalize value to 0
				float range = m_max - m_min;
				float nvalue = range > 0.0f ? (m_value - m_min) / range : 0.0f;

				SizeF thumbSize = m_thumb->GetSize();

				// horizontal orientation
				if (m_horizontal)
				{
					float x = (GetSize().width - thumbSize.width) * nvalue;
					m_thumb->SetPosition({ x, 0 });
				}
				// vertical orientation
				else
				{
					float y = (GetSize().height - thumbSize.height) * nvalue;
					m_thumb->SetPosition({ 0, y });
				}
			}

			void UpdateThumbSize()
			{
				// if horizontal orientation, get slider height. otherwise, get width
				float thickness = m_horizontal ? GetHeight() : GetWidth();

				// get the slider length. 
				float sliderLength = m_horizontal ? GetWidth() : GetHeight();

				// if clamp thumb length within slider length, if needed
				float length = m_thumbLength < sliderLength ? m_thumbLength : sliderLength;

				SizeF thumbSize
				{
					m_horizontal ? length : thickness,
					m_horizontal ? thickness : length,
				};

				m_thumb->SetSize(thumbSize);
			}

			void UpdateValueFromPosition(const PositionF& position)
			{
				// translate the clicked position (world position) to slider's local coordinate
				PositionF local = position - GetAbsolutePosition();

				float thumbLength = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;

				// get length of the slider. this is the length it can move, so subtract thumb length
				float length = m_horizontal ? GetSize().width - m_thumb->GetSize().width : GetSize().height - m_thumb->GetSize().height;

				// normalize the value of the position based on slider length
				float nvalue = length <= 0.0f ? 0.0f : (m_horizontal ? local.x - thumbLength / 2.0f : local.y - thumbLength / 2.0f) / length;

				// if in case position is outside slider extents, clamp it
				nvalue = std::clamp<float>(nvalue, 0.0f, 1.0f);

				// convert it into value based on range 
				float value = m_min + (m_max - m_min) * nvalue;

				value = Snap(value);

				// finally we set value
				Value(value);
			}

			float Snap(float value) const
			{
				if (m_steps <= 1) return m_min;

				int steps = m_steps - 1;

				float stepSize = (m_max - m_min) / steps;

				value = std::round((value - m_min) / stepSize);
				value *= stepSize;
				value += m_min;

				return value;
			}

		public:
			engine::event::Event<float> OnChange;

			Slider(float min, float max, float thumbLength) :
				m_min(min),
				m_max(max),
				m_thumbLength(thumbLength),
				m_value(min),
				m_horizontal(true),
				m_isDragging(false),
				m_steps(0)
			{
				m_moveBehavior = MoveBehavior::None;

				std::unique_ptr<Thumb> thumb = std::make_unique<Thumb>();
				m_thumb = thumb.get();

				AddChild(std::move(thumb));

			}

			void Horizontal(bool enable)
			{
				if (m_horizontal != enable)
				{
					m_horizontal = enable;

					UpdateThumbSize();

					UpdateThumbPosition();
				}
			}

			bool Horizontal() const
			{
				return m_horizontal;
			}

			void SetThumbLength(float length)
			{
				m_thumbLength = length;

				UpdateThumbSize();

				UpdateThumbPosition();
			}

			void Min(float min)
			{
				m_min = min;

				// range changed, we might need to update value if it gets clamped
				Value(m_value);

				// value might not have changed, but thumb position might change with new range
				UpdateThumbPosition();
			}

			void Max(float max)
			{
				m_max = max;

				// range changed, we might need to update value if it gets clamped
				Value(m_value);

				// value might not have changed, but thumb position might change with new range
				UpdateThumbPosition();
			}

			float Min() const
			{
				return m_min;
			}

			float Max() const
			{
				return m_max;
			}

			void Value(float value)
			{
				// always clamp to min if min happens to be larger than max
				if (m_min > m_max)
				{
					value = m_min;
				}
				// otherwise make sure to clamp within range
				else
				{
					value = std::clamp<float>(value, m_min, m_max);
				}

				// if new value same as current, no change, no update, no notification needed
				if (m_value == value)
				{
					return;
				}

				// we're ready to set new value
				m_value = value;

				// update thumb position
				UpdateThumbPosition();

				// fire up event
				OnChange(m_value);
			}

			float Value() const
			{
				return m_value;
			}

			void SetStepSize(float size)
			{
				m_steps = size > 0 ?
					static_cast<int>(std::round((m_max - m_min) / size)) + 1 :
					1;

				// snap the current value and update value. this will also possibly update thumb position if needed
				Value(Snap(m_value));
			}

			void SetStepCount(int count)
			{
				m_steps = count;

				// snap the current value and update value. this will also possibly update thumb position if needed
				Value(Snap(m_value));
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawSlider(*this, context);
			}
		};

		class Switch : public Widget
		{
		private:
			BoundRef<bool> m_checked;
			bool m_pressed;

		public:
			Switch() :
				m_pressed(false)
			{
				m_moveBehavior = MoveBehavior::None;
			}

			event::Event<bool> Click;

			void Bind(bool& checked)
			{
				m_checked.Bind(checked);
			}

			void Bind(
				std::function<bool()> getter,
				std::function<void(const bool&)> setter
			)
			{
				m_checked.Bind(getter, setter);
			}

			void Toggle()
			{
				m_checked = !m_checked;
			}

			void TurnOn()
			{
				if (!m_checked) m_checked = true;
			}

			void TurnOff()
			{
				if (m_checked) m_checked = false;
			}

			void OnMouseDown(const PositionF& position) override
			{
				m_pressed = true;
			}

			bool IsOn() const
			{
				return m_checked;
			}

			void OnMouseUp(const PositionF& position) override
			{
				if (!m_pressed) return;

				m_pressed = false;

				// did the mouse release occur over this button? if not, then this mouse up is not for us. ignore
				if (!Contains(position)) return;

				Toggle();

				// handle click event
				Click(m_checked);
			}
		};

		class CheckBox : public Switch
		{
		private:

		public:
			CheckBox()
			{
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawCheckBox(*this, context);
			}
		};

		class RadioButton : public Switch
		{
		private:

		public:
			RadioButton()
			{
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawRadioButton(*this, context);
			}
		};

		class ScrollBar : public Widget
		{
		private:
			float m_contentLength;
			float m_viewportLength;
			float m_offset; // current scroll position
			bool m_horizontal;
			Widget* m_thumb;
			bool m_isDragging;
			float m_minThumbLength;

		protected:
			bool CanScroll() const
			{
				return m_contentLength > m_viewportLength;
			}

			void OnMouseDown(const PositionF& pos) override
			{
				// if cannot scroll e.g. content smaller than viewport, no need to drag
				if (!CanScroll()) return;

				// did we clicked on thumb or track?
				float lpos = (m_horizontal ? pos.x : pos.y) - (m_horizontal ? GetAbsolutePosition().x : GetAbsolutePosition().y);
				float thumbSize = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;
				float thumbPos = m_horizontal ? m_thumb->GetPosition().x : m_thumb->GetPosition().y;

				// if click on track, move thumb to that position, and start dragging
				if (lpos < thumbPos || lpos > thumbPos + thumbSize)
				{
					float newThumbPos = lpos - thumbSize / 2.0f;
					m_thumb->SetPosition(m_horizontal ? PositionF{ newThumbPos,  m_thumb->GetPosition().y } : PositionF{ m_thumb->GetPosition().x, newThumbPos });
				}

				m_isDragging = true;
				m_thumb->MouseDown(pos);
			}

			void OnMouseUp(const PositionF&) override { m_isDragging = false; }

			void OnMouseMove(const PositionF& pos) override
			{
				if (m_isDragging)
				{
					m_thumb->MouseMove(pos);
				}
			}

			void UpdateThumbSize()
			{
				float trackLength = m_horizontal ? GetWidth() : GetHeight();
				float thickness = m_horizontal ? GetHeight() : GetWidth();

				// get ratio between viewport and content length. this is basically the normalized length of the thumb. 
				// this can be > 1 if content is smaller than viewport
				// if either content or viewport is 0, set ratio to 1.0f so that thumb will be full length of scrollbar, no scrolling needed
				float ratio = m_contentLength > 0 ? m_viewportLength > 0 ? m_viewportLength / m_contentLength : 1.0f : 1.0f;

				// here when calculating the actual length of the thumb, we clamp to tracklength so even if ratio > 1, we don't end up with thumb bigger than scroll bar
				float length = std::clamp(ratio * trackLength,
					// we're comparing value between m_minThumbLength and trackLength. between m_minThumbLength and trackLength, use smaller for min value and bigger for max value. 
					// this is a must because if min value is bigger than max value, std::clamp will throw an exception
					m_minThumbLength > trackLength ? trackLength : m_minThumbLength,
					trackLength > m_minThumbLength ? trackLength : m_minThumbLength
				); // clamp min size

				SizeF thumbSize{
					m_horizontal ? length : thickness,
					m_horizontal ? thickness : length
				};
				m_thumb->SetSize(thumbSize);
			}

			void UpdateThumbPosition()
			{
				// tracklength is the length of the scrollbar in pixels. it will be used to calculate the index size
				float trackLength = m_horizontal ? GetSize().width : GetSize().height;

				// how much pixels in tracklength does each content occupies? that is index size
				// if content is less than viewport, set it to 0 as we doin't need to scroll
				// if content is 0, then there is nothing to scroll. handle this because if viewport is negative and content is 0, the previous condition might pass a content being 0
				// if viewport is 0, it could mean content is infinitely large compared to viewport, so we don't need to scroll either. set it to 0
				float indexSize =
					m_viewportLength == 0.0f ? 0 :
					m_contentLength == 0.0f ? 0 :
					m_contentLength <= m_viewportLength ? 0 :
					trackLength / m_contentLength;

				// now let's calculate the thumb position to snap it to the index.
				float thumbPos = m_offset * indexSize;
				m_thumb->SetPosition(m_horizontal ? PositionF{ thumbPos, 0 } : PositionF{ 0, thumbPos });
			}

			void OnSizeChanged(const SizeF&, const SizeF&) override
			{
				UpdateThumbSize();
				UpdateThumbPosition();
			}

		public:
			engine::event::Event<float> Scroll;

			ScrollBar(float contentLength, float viewportLength, bool isHorizontal) :
				m_contentLength(contentLength),
				m_viewportLength(viewportLength),
				m_offset(0),
				m_horizontal(isHorizontal),
				m_isDragging(false),
				m_minThumbLength(16.0f)
			{
				// this scrollbar is not movable. it is a static widget that can only be moved by dragging the thumb
				m_moveBehavior = MoveBehavior::None;

				// create thumb and add it as child. we will use this thumb to handle dragging and scrolling
				std::unique_ptr<Thumb> thumb = std::make_unique<Thumb>(m_horizontal);
				m_thumb = thumb.get();
				AddChild(std::move(thumb));

				// handle thumb movement. when thumb is moved, we will calculate the new offset based on thumb position and content length, viewport length, and scrollbar length
				m_thumb->OnMove += [this](const PositionF& newPos)
					{
						// trackLength is the length scrollbar can move. so this must be length of scrollbar minus length of thumb. this is used to calculate the index size
						float trackLength = (m_horizontal ? GetSize().width : GetSize().height) - (m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height);

						// scroll size is the min/max range of value that scrollbar can scroll. content is the total range, while viewport is the viewable range
						// content is the size of the data that can be viewed. viewport is the size of the data that is viewable.
						// if viewport is smaller than content, then scrolling is required 
						float scrollSize = m_contentLength - m_viewportLength;

						// given trackLength which is the actual scroll range of scrollbar in pixel, and scrollSize which is the range it can scroll,
						// scrollIndexSize is the size in pixel per every value the scrollbar can scroll
						// if scrollsize is 0 or negative, then scrolling should not happen
						float scrollIndexSize = scrollSize > 0 ? trackLength / scrollSize : 0;

						// get thumb position. this is based on thumb widget's position and the scrollbar's orientation
						float thumbPos = m_horizontal ? newPos.x : newPos.y;

						// if scroll index size is 0, then scrolling should not happen
						float index = scrollIndexSize == 0 ? 0 : std::floor(thumbPos / scrollIndexSize + 0.5f);

						// clamp index such that its value can only be between 0 and (m_contentLength - m_viewportLength)
						index = std::clamp<float>(index, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

						// now let's calculate the thumb position to snap it to the index.
						thumbPos = index * scrollIndexSize;
						m_thumb->SetPosition(m_horizontal ? PositionF{ thumbPos, 0 } : PositionF{ 0, thumbPos });

						// finally, if the calculated index is same as current offset, we don't have to do anything
						// note that offset is just another name for index.
						if (m_offset == index) return;
						m_offset = index;
						Scroll(m_offset);
					};
			}

			void SetOffset(float offset)
			{
				// clamp offset to be within 0 and (contentLength - viewportLength). if content is smaller than viewport, set offset to 0
				offset = std::clamp(offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

				// if thumb position did not change, no need to update and invoke scroll events
				if (m_offset == offset) return;

				m_offset = offset;
				UpdateThumbPosition();
				Scroll(m_offset);
			}

			float Offset() const { return m_offset; }

			void SetContentLength(float length)
			{
				// cannot be negative content length. if negative, set to 0
				m_contentLength = std::max<float>(0.0f, length);

				// refresh offset in case it is out of range now due to content length change. if content is smaller than viewport, set offset to 0
				m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

				// update thumb size and position based on possibly new content length and offset
				UpdateThumbSize();
				UpdateThumbPosition();
			}

			void SetViewportLength(float length)
			{
				// cannot be negative viewport length. if negative, set to 0
				m_viewportLength = std::max<float>(0.0f, length);

				// refresh offset in case it is out of range now due to content length change. if content is smaller than viewport, set offset to 0
				m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

				// update thumb size and position based on possibly new content length and offset
				UpdateThumbSize();
				UpdateThumbPosition();
			}

			float GetContentLength() const { return m_contentLength; }
			float GetViewportLength() const { return m_viewportLength; }

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawScrollBar(*this, context);
			}
		};

		class Content : public Widget
		{
		private:
		public:
			Content()
			{
				m_moveBehavior = MoveBehavior::Free;
				m_droppable = false;
				m_focusable = false;
			}

			Content(bool movable)
			{
				m_moveBehavior = movable ? MoveBehavior::Free : MoveBehavior::None;
				m_droppable = false;
				m_focusable = false;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawContent(*this, context);
			}
		};

		class Grip : public Widget
		{
		protected:

		public:
			Grip(bool MoveHorizontal, bool MoveVertical)
			{
				m_moveBehavior = (MoveHorizontal && MoveVertical) ? MoveBehavior::Free :
					(MoveHorizontal && !MoveVertical) ? MoveBehavior::Horizontal :
					(!MoveHorizontal && MoveVertical) ? MoveBehavior::Vertical :
					MoveBehavior::None;

				m_droppable = false;
				m_focusable = false;
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawGrip(*this, context);
			}
		};

		// a frame the can be resized when dragging its edge/corner grips
		// it has a min size that clamps to it when resizing the frame via grips
		class ResizeableFrame : public Widget
		{
		private:
			// resize grip components
			Grip* m_leftResizeGrip = nullptr;
			Grip* m_rightResizeGrip = nullptr;
			Grip* m_topResizeGrip = nullptr;
			Grip* m_bottomResizeGrip = nullptr;
			Grip* m_topLeftResizeGrip = nullptr;
			Grip* m_topRightResizeGrip = nullptr;
			Grip* m_bottomLeftResizeGrip = nullptr;
			Grip* m_bottomRightResizeGrip = nullptr;

			Widget* m_content = nullptr;

			// resize grip thickness
			float m_borderSize;

			// min size when resizing through grips
			SizeF m_minResize;

			// resizing trackers
			PositionF m_beginPosition;
			SizeF m_beginSize;

		protected:
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				UpdateLayout();
			}

			void UpdateLayout()
			{
				UpdateRightGripLayout();
				UpdateLeftGripLayout();
				UpdateBottomGripLayout();
				UpdateTopGripLayout();
				UpdateTopLeftGripLayout();
				UpdateTopRightGripLayout();
				UpdateBottomLeftGripLayout();
				UpdateBottomRightGripLayout();

				UpdateContentLayout();

				ContentAreaSizeChanged(m_content->GetSize());
			}

			void UpdateContentLayout()
			{
				m_content->SetPosition({ m_borderSize, m_borderSize });
				m_content->SetSize({ GetSize().width - m_borderSize * 2, GetSize().height - m_borderSize * 2 });
			}

			void UpdateTopRightGripLayout()
			{
				// resize and reposition right grip control to occupy top-right corner of the frame with border size as thickness
				m_topRightResizeGrip->SetPosition({ m_size.width - m_borderSize, 0 });
				m_topRightResizeGrip->SetSize({ m_borderSize, m_borderSize });
			}

			void UpdateTopGripLayout()
			{
				// resize and reposition right grip control to occupy top edge of the frame with border size as thickness
				m_topResizeGrip->SetPosition({ m_borderSize, 0 });
				m_topResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });
			}

			void UpdateRightGripLayout()
			{
				// resize and reposition right grip control to occupy right edge of the frame with border size as thickness
				m_rightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_borderSize });
				m_rightResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });
			}

			void UpdateTopLeftGripLayout()
			{
				// resize and reposition right grip control to occupy top-left corner of the frame with border size as thickness
				m_topLeftResizeGrip->SetPosition({ 0, 0 });
				m_topLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });
			}

			void UpdateLeftGripLayout()
			{
				// resize and reposition right grip control to occupy left edge of the frame with border size as thickness
				m_leftResizeGrip->SetPosition({ 0, m_borderSize });
				m_leftResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });
			}

			void UpdateBottomRightGripLayout()
			{
				// resize and reposition right grip control to occupy bottom-right corner of the frame with border size as thickness
				m_bottomRightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_size.height - m_borderSize });
				m_bottomRightResizeGrip->SetSize({ m_borderSize, m_borderSize });
			}

			void UpdateBottomGripLayout()
			{
				// resize and reposition right grip control to occupy bottom edge of the frame with border size as thickness
				m_bottomResizeGrip->SetPosition({ m_borderSize, m_size.height - m_borderSize });
				m_bottomResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });
			}

			void UpdateBottomLeftGripLayout()
			{
				// resize and reposition right grip control to occupy bottom-left corner of the frame with border size as thickness
				m_bottomLeftResizeGrip->SetPosition({ 0, m_size.height - m_borderSize });
				m_bottomLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });
			}

			SizeF ClampSize(const SizeF& size) const
			{
				return
				{
					std::max<float>(size.width,  m_minResize.width),
					std::max<float>(size.height, m_minResize.height)
				};
			}

		public:

			ResizeableFrame(float borderSize = 20.0f, const SizeF& minSize = { 200, 200 }) :
				m_borderSize(borderSize),
				m_minResize(minSize)
			{
				// create our grip widgets
				{
					// left grip
					std::unique_ptr<Grip> widget = std::make_unique<Grip>(true, false);
					m_leftResizeGrip = widget.get();
					AddChild(std::move(widget));

					// right grip
					widget = std::make_unique<Grip>(true, false);
					m_rightResizeGrip = widget.get();
					AddChild(std::move(widget));

					// top grip
					widget = std::make_unique<Grip>(false, true);
					m_topResizeGrip = widget.get();
					AddChild(std::move(widget));

					// bottom grip
					widget = std::make_unique<Grip>(false, true);
					m_bottomResizeGrip = widget.get();
					AddChild(std::move(widget));

					// top-left grip
					widget = std::make_unique<Grip>(true, true);
					m_topLeftResizeGrip = widget.get();
					AddChild(std::move(widget));

					// top-right grip
					widget = std::make_unique<Grip>(true, true);
					m_topRightResizeGrip = widget.get();
					AddChild(std::move(widget));

					// bottom-left grip
					widget = std::make_unique<Grip>(true, true);
					m_bottomLeftResizeGrip = widget.get();
					AddChild(std::move(widget));

					// bottom-right grip
					widget = std::make_unique<Grip>(true, true);
					m_bottomRightResizeGrip = widget.get();
					AddChild(std::move(widget));

					std::unique_ptr<Widget> client = std::make_unique<Content>();
					m_content = client.get();
					AddChild(std::move(client));
				}

				// begin drag lambda is same for all grips, so we define one here and assign to all grips
				{
					auto capture = [&](const Widget::DragEventArgs&)
						{
							m_beginPosition = GetPosition();
							m_beginSize = GetSize();
						};
					m_bottomRightResizeGrip->OnDragBegin += capture;
					m_topLeftResizeGrip->OnDragBegin += capture;
					m_bottomResizeGrip->OnDragBegin += capture;
					m_bottomLeftResizeGrip->OnDragBegin += capture;
					m_topRightResizeGrip->OnDragBegin += capture;
					m_topResizeGrip->OnDragBegin += capture;
					m_leftResizeGrip->OnDragBegin += capture;
					m_rightResizeGrip->OnDragBegin += capture;
				}

				// everytime grip moves, we make sure it always stay at the position relative to frame all the time.
				// sometimes when you drag a grip, and the drag change is same as before, resizeable frame size does not change. 
				// this will not trigger resizeableframe's OnSizeChange and therefore will not exacute LayoutUpdate. the grip will then be out of position.
				// repositioning it here ensures that everytime grip moves, it will be repositioned back to its supposed location relative to resizeableframe
				// the impact though is that it can reposition the grip multiple times per move. if it so happens resizeableframe is resized, it will
				// execute LayoutUpdate which will reposition the grip. then this handle will again reposition it. it won't cause recursive chain since
				// SetPosition() is guarded. but it costs CPU execution time as grip's SetPosition() can be called more than once.
				{
					m_topRightResizeGrip->OnMove += [&](const PositionF& pos) { UpdateTopRightGripLayout(); };
					m_topResizeGrip->OnMove += [&](const PositionF& pos) { UpdateTopGripLayout(); };
					m_topLeftResizeGrip->OnMove += [&](const PositionF& pos) { UpdateTopLeftGripLayout(); };
					m_rightResizeGrip->OnMove += [&](const PositionF& pos) { UpdateRightGripLayout(); };
					m_leftResizeGrip->OnMove += [&](const PositionF& pos) { UpdateLeftGripLayout(); };
					m_bottomRightResizeGrip->OnMove += [&](const PositionF& pos) { UpdateBottomRightGripLayout(); };
					m_bottomResizeGrip->OnMove += [&](const PositionF& pos) { UpdateBottomGripLayout(); };
					m_bottomLeftResizeGrip->OnMove += [&](const PositionF& pos) { UpdateBottomLeftGripLayout(); };
				}

				// we also track content drag in case content is draggable, we bubble up movement to the frame and make content stationary
				{
					m_content->OnDragBegin += [&](const Widget::DragEventArgs& args)
						{
							MouseDown(args.currentPosition);
						};

					m_content->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							MouseMove(args.currentPosition);
							UpdateLayout();
						};

					m_content->OnDragEnd += [&](const Widget::DragEventArgs& args)
						{
							MouseUp(args.currentPosition);
						};
				}

				// we track grips' drag. we update resizeableframe's position and size depending on grip's drag movement
				{
					// bottom-right grip handlers
					m_bottomRightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							SetSize(ClampSize(
								{
									m_beginSize.width + delta.x,
									m_beginSize.height + delta.y
								}));
						};

					//  top-left grip handlers
					m_topLeftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaX = m_beginSize.width - m_minResize.width;

							// the delta width will be clamped to max allowed width
							delta.x = std::min<float>(delta.x, maxDeltaX);

							// when dragging top grip, if moving downwards, we are reducing the height of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaY = m_beginSize.height - m_minResize.height;

							// the delta width will be clamped to max allowed width
							delta.y = std::min<float>(delta.y, maxDeltaY);

							SetPosition(m_beginPosition + delta);

							SetSize(
								{
									m_beginSize.width - delta.x,
									m_beginSize.height - delta.y,
								});
						};

					//  bottom-left grip handlers
					m_bottomLeftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaX = m_beginSize.width - m_minResize.width;

							// the delta width will be clamped to max allowed width
							delta.x = std::min<float>(delta.x, maxDeltaX);

							SetPosition(
								{
									m_beginPosition.x + delta.x,
									m_beginPosition.y
								});

							SetSize(ClampSize(
								{
									m_beginSize.width - delta.x,
									m_beginSize.height + delta.y,
								}));
						};

					//  top-right grip handlers
					m_topRightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							// when dragging top-right grip, if moving downwards, we are reducing the height of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaY = m_beginSize.height - m_minResize.height;

							// the delta height will be clamped to max allowed height
							delta.y = std::min<float>(delta.y, maxDeltaY);

							SetPosition(
								{
									m_beginPosition.x,
									m_beginPosition.y + delta.y
								});

							SetSize(ClampSize(
								{
									m_beginSize.width + delta.x,
									m_beginSize.height - delta.y,
								}));
						};

					//  top grip handlers
					m_topResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							// when dragging top grip, if moving downwards, we are reducing the height of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaY = m_beginSize.height - m_minResize.height;

							// the delta width will be clamped to max allowed width
							delta.y = std::min<float>(delta.y, maxDeltaY);

							SetPosition(
								{
									m_beginPosition.x,
									m_beginPosition.y + delta.y
								});

							SetSize(
								{
									m_beginSize.width,
									m_beginSize.height - delta.y,
								});
						};

					//  left grip handlers
					m_leftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							VecF delta = args.Delta();

							// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
							// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
							float maxDeltaX = m_beginSize.width - m_minResize.width;

							// the delta width will be clamped to max allowed width
							delta.x = std::min<float>(delta.x, maxDeltaX);

							SetPosition(
								{
									m_beginPosition.x + delta.x,
									m_beginPosition.y
								});

							SetSize(
								{
									m_beginSize.width - delta.x,
									m_beginSize.height,
								});
						};

					//  right grip handlers
					m_rightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							SetSize(ClampSize(
								{
									m_beginSize.width + args.Delta().x,
									m_beginSize.height,
								}));
						};

					//  bottom grip handlers
					m_bottomResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
						{
							SetSize(ClampSize(
								{
									m_beginSize.width,
									m_beginSize.height + args.Delta().y,
								}));
						};
				}
			}


			void SetMinResize(const SizeF& size)
			{
				m_minResize = size;
			}

			void SetBorderSize(float size)
			{
				m_borderSize = size;
			}

			engine::event::Event<const SizeF&> ContentAreaSizeChanged;

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawResizeableFrame(*this, context);
			}

			void AddContent(std::unique_ptr<Widget> widget)
			{
				m_content->AddChild(std::move(widget));
			}

			SizeF GetContentsize() const
			{
				return m_content->GetSize();
			}

			SizeF GetContentAreaSize() const
			{
				return SizeF{ GetSize().width - m_borderSize * 2, GetSize().height - m_borderSize * 2 };
			}
		};

		class ViewPort : public Widget
		{
		protected:
			Widget* m_content;

			// this method ensures that content's position is always within the viewport's bounds. 
			// if content's position is outside the viewport, it will be moved back to the nearest position within the viewport
			void UpdateContentPosition()
			{
				// if content's position is > 0,0 then move it back to 0, 0
				PositionF position = m_content->GetPosition();
				SizeF size = m_content->GetSize();

				bool updatePos = false;

				// calculate the min position of content when it is dragged to the left and up. 
				// content must not be dragged left and up beyond 0,0 while its bottom-right edge are already inside teh viewport
				float minX = std::min<float>(0.0f, GetSize().width - size.width);
				float minY = std::min<float>(0.0f, GetSize().height - size.height);

				// Clamp X: position must stay between minX and 0.0f
				float clampedX = std::clamp(position.x, minX, 0.0f);
				if (position.x != clampedX)
				{
					position.x = clampedX;
					updatePos = true;
				}

				// Clamp Y: position must stay between minY and 0.0f
				float clampedY = std::clamp(position.y, minY, 0.0f);
				if (position.y != clampedY)
				{
					position.y = clampedY;
					updatePos = true;
				}

				if (updatePos)
				{
					m_content->SetPosition(position);
				}
			}

			// handler for when viewport's size changes. we need to ensure that content's position is still within the viewport's bounds
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				UpdateContentPosition();
			}

			// event handler for when content's size changes. we need to ensure that content's position is still within the viewport's bounds
			void OnContentSizeChanged(const SizeF& size)
			{
				UpdateContentPosition();

				// bubble up the event to notify that content's size has changed. this is useful for scroll bars to update their thumb size and position
				ContentSizeChanged(size);
			}

			void OnContentMove(const PositionF& pos)
			{
				// When content moves, ensure it remains within the viewport bounds.
				// 
				// OnMove is only raised when content's position really changed.
				// Update the viewport's layout to ensure content stays within the bounds of viewport's area.
				// Updating the layout may result in setting content's position again which will result in raising OnMove.
				// As OnMove will only be raised  when content's position really changed, this will not result in recursive loop
				//
				// also note that since we are monitoring content's move event, we don't need to monitor its drag event as dragging will 
				// also eventually set position of content and will raise OnMove
				UpdateContentPosition();

				// fire scroll event to notify that content has moved and viewport's offset has changed
				Scroll(GetOffset());
			}

		public:
			engine::event::Event<const VecF&> Scroll;
			engine::event::Event<const SizeF&> ContentSizeChanged;
			engine::event::Event<Widget*> ContentChanged;

			ViewPort()
				:m_content(nullptr)
			{
				// this widget should not be movable. it also should not be focusable because it is a container for a content that is the actual widget this represents
				m_moveBehavior = MoveBehavior::None;
				m_droppable = false;
				m_focusable = false;

				// create a default content widget and set it as the viewport's content
				std::unique_ptr<Widget> content = std::make_unique<Widget>();
				SetContent(std::move(content));
			}

			void SetContent(std::unique_ptr<Widget> content)
			{
				// ensure content is not null. we don't allow null content as viewport must always have a content widget
				if (!content) throw std::invalid_argument("content cannot be null");

				// if we have an existing content, we need to remove it and unsubscribe from its events before setting the new content
				if (m_content)
				{
					// let's unsubscribe from current content's events before removing it.
					m_content->OnResize -= engine::event::Handler(this, &ViewPort::OnContentSizeChanged);
					m_content->OnMove -= engine::event::Handler(this, &ViewPort::OnContentMove);

					// this will destroy the content widget and all its children. so beware, this is permanent
					RemoveChild(m_content);
					m_content = nullptr;
				}

				// set new content
				m_content = content.get();
				AddChild(std::move(content));

				// subscribe to new content's events
				m_content->OnResize += engine::event::Handler(this, &ViewPort::OnContentSizeChanged);
				m_content->OnMove += engine::event::Handler(this, &ViewPort::OnContentMove);

				// we don't know what is the new content's size and position, so we need to ensure that it is within the viewport's bounds
				UpdateContentPosition();

				// fire event to notify that content has changed
				ContentChanged(m_content);
			}

			void SetContentSize(const SizeF& size)
			{
				m_content->SetSize(size);
			}

			SizeF GetContentSize() const
			{
				return m_content->GetSize();
			}

			VecF GetOffset() const
			{
				return PositionF{ 0,0 } - m_content->GetPosition();
			}

			void SetOffset(const VecF& offset)
			{
				// Setting the viewport offset is done by moving the content in the
				// opposite direction relative to the viewport.
				//
				// This ultimately calls m_content->SetPosition().
				//
				// If the position actually changes, Widget::SetPosition() will fire
				// the content's OnMove event. ViewPort listens to that event and
				// performs UpdateLayout() to enforce viewport bounds and any other
				// scrolling rules.
				//
				// Widget::SetPosition() is guarded against assigning the same value,
				// preventing redundant notifications and avoiding recursive update
				// loops when UpdateLayout() performs corrective repositioning.
				PositionF pos = PositionF{ 0,0 } - offset;
				m_content->SetPosition(pos);
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawViewPort(*this, context);
			}

			void AddContent(std::unique_ptr<Widget> content)
			{
				m_content->AddChild(std::move(content));
			}
		};

		class ScrollView : public Widget
		{
		protected:
			ViewPort* m_viewport;
			ScrollBar* m_hScrollBar;
			ScrollBar* m_vScrollBar;
			bool m_autoHideScrollBars;

			float m_scrollSize;
			float m_borderSize;

		protected:

			// this will update thumb positions of the scroll bars
			void UpdateThumbPositions()
			{
				VecF offset = m_viewport->GetOffset();
				m_hScrollBar->SetOffset(offset.x);
				m_vScrollBar->SetOffset(offset.y);
			}

			// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarContentSize()
			{
				SizeF contentSize = m_viewport->GetContentSize();

				m_hScrollBar->SetContentLength(contentSize.width);
				m_vScrollBar->SetContentLength(contentSize.height);
			}

			// this will update the viewport size of scroll bars.  scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarViewportSize()
			{
				SizeF viewportSize = m_viewport->GetSize();

				m_hScrollBar->SetViewportLength(viewportSize.width);
				m_vScrollBar->SetViewportLength(viewportSize.height);
			}

			void UpdateLayout()
			{
				if (m_autoHideScrollBars)
				{
					// let's assume scroll bars are not needed first
					bool hScrollBarVisible = false;
					bool vScrollBarVisible = false;

					// since we assume there are no scrollbars so we also assume viewport occupies the whole scrollview
					SizeF viewportSize = GetSize();

					// if content.size < viewport.size, no need to do anything
					SizeF contentSize = m_viewport->GetContentSize();

					// we need to do a few passes to check if either or both scrollbars are needed
					while (
						(contentSize.height > viewportSize.height && !vScrollBarVisible) ||
						(contentSize.height <= viewportSize.height && vScrollBarVisible) ||
						(contentSize.width > viewportSize.width && !hScrollBarVisible) ||
						(contentSize.width <= viewportSize.width && hScrollBarVisible)
						)
					{
						if (contentSize.width > viewportSize.width)
						{
							hScrollBarVisible = true;
							viewportSize.height = GetSize().height - m_borderSize - m_scrollSize;
						}
						else
						{
							hScrollBarVisible = false;
							viewportSize.height = GetSize().height;
						}


						if (contentSize.height > viewportSize.height)
						{
							vScrollBarVisible = true;
							viewportSize.width = GetSize().width - m_borderSize - m_scrollSize;
						}
						else
						{
							vScrollBarVisible = false;
							viewportSize.width = GetSize().width;
						}
					}

					// now let's hide or show vertical scrollbar 
					if (vScrollBarVisible) m_vScrollBar->Show();
					else m_vScrollBar->Hide();

					// now let's hide or show horizontal scrollbar 
					if (hScrollBarVisible) m_hScrollBar->Show();
					else m_hScrollBar->Hide();

					// update viewport size and position
					m_viewport->SetPosition({ 0,0 });
					m_viewport->SetSize(viewportSize);
				}
				else
				{
					// if scrollbars are always visible...
					m_vScrollBar->Show();
					m_hScrollBar->Show();

					m_viewport->SetPosition({ 0,0 });
					m_viewport->SetSize({
							GetSize().width - m_borderSize - m_scrollSize,
							GetSize().height - m_borderSize - m_scrollSize
						});
				}

				// update horizontal scrollbar size and position
				m_hScrollBar->SetPosition({ 0, GetSize().height - m_scrollSize });
				m_hScrollBar->SetSize(
					{
						GetSize().width - (m_vScrollBar->IsVisible() ? m_scrollSize : 0.0f),
						m_scrollSize,
					}
					);

				// update vertical scrollbar size and position
				m_vScrollBar->SetPosition({ GetSize().width - m_scrollSize, 0 });
				m_vScrollBar->SetSize(
					{
						m_scrollSize,
						GetSize().height - (m_hScrollBar->IsVisible() ? m_scrollSize : 0.0f),
					}
					);

				UpdateScrollBarContentSize();
				UpdateScrollBarViewportSize();
			}

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				UpdateLayout();
			}

		public:

			engine::event::Event<const VecF&> Scroll;
			engine::event::Event<Widget*> ContentChanged;
			engine::event::Event<const SizeF&> ViewPortResized;

			ScrollView(bool autoHideScrollBars = true, float scrollSize = 20.0f, float borderSize = 2.0f)
				: m_scrollSize(scrollSize)
				, m_borderSize(borderSize)
				, m_viewport(nullptr)
				, m_hScrollBar(nullptr)
				, m_vScrollBar(nullptr)
				, m_autoHideScrollBars(autoHideScrollBars)
			{
				m_moveBehavior = MoveBehavior::None;

				std::unique_ptr<ViewPort> viewport = std::make_unique<ViewPort>();
				m_viewport = viewport.get();
				AddChild(std::move(viewport));

				std::unique_ptr<ScrollBar> hScrollBar = std::make_unique<ScrollBar>(0.0f, 0.0f, true);
				m_hScrollBar = hScrollBar.get();
				AddChild(std::move(hScrollBar));

				std::unique_ptr<ScrollBar> vScrollBar = std::make_unique<ScrollBar>(0.f, 0.0f, false);
				m_vScrollBar = vScrollBar.get();
				AddChild(std::move(vScrollBar));

				// listen to viewport's scroll event. when viewport's content moves or scrolls, we need to update our scrollbar's thumb positions
				m_viewport->Scroll += [&](const VecF& offset)
					{
						// this will update thumb positions of the scroll bars
						UpdateThumbPositions();

						Scroll(offset);
					};

				m_viewport->ContentSizeChanged += [&](const SizeF& size)
					{
						// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
						UpdateScrollBarContentSize();
						UpdateLayout();
					};

				m_viewport->ContentChanged += [&](Widget* content)
					{
						// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
						UpdateScrollBarContentSize();
						UpdateLayout();

						// bubble up the event to our own OnContentChange event
						ContentChanged(content);
					};

				// bubble up viewport's resize event to our own OnViewPortResize event. 
				// this is useful for external content that needs to resize itself when viewport resizes.
				m_viewport->OnResize += [&](const SizeF& size)
					{
						ViewPortResized(size);
					};

				// handler for when horizontal scrollbar scrolls. only viewport's content position changes here. layout remains the same.
				m_hScrollBar->Scroll += [&](float offset)
					{
						VecF currOffset = m_viewport->GetOffset();
						currOffset.x = offset;
						m_viewport->SetOffset(currOffset);
					};

				// handler for when vertical scrollbar scrolls. only viewport's content position changes here. layout remains the same.
				m_vScrollBar->Scroll += [&](float offset)
					{
						VecF currOffset = m_viewport->GetOffset();
						currOffset.y = offset;
						m_viewport->SetOffset(currOffset);
					};

			}

			void SetContentSize(const SizeF& size)
			{
				m_viewport->SetContentSize(size);
			}

			void SetContent(std::unique_ptr<Widget> content)
			{
				m_viewport->SetContent(std::move(content));
			}

			// should this be called View? or Client? what's the best name for this?
			SizeF GetViewPortSize() const
			{
				return m_viewport->GetSize();
			}

			VecF GetViewPortOffset() const
			{
				return m_viewport->GetOffset();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin) context.skin->DrawScrollView(*this, context);
			}

		};

		class Stack : public Widget
		{
		protected:
			bool m_vertical;
			float m_borderSize;

			void UpdateLayout()
			{
				SizeF size;
				float accumulatedPos = 0.0f;
				bool first = true;
				ForEachChild([&](Widget* widget)
					{
						PositionF pos
						{
							m_vertical ? 0.0f : (accumulatedPos + (first ? 0.0f : m_borderSize)),
							m_vertical ? (accumulatedPos + (first ? 0.0f : m_borderSize)) : 0.0f
						};
						widget->SetPosition(pos);

						accumulatedPos += (m_vertical ? widget->GetHeight() : widget->GetWidth());
						accumulatedPos += (first ? 0.0f : m_borderSize);

						if (m_vertical)
						{
							size.width = size.width < widget->GetWidth() ? widget->GetWidth() : size.width;
							size.height = accumulatedPos;
						}
						else
						{
							size.height = size.height < widget->GetHeight() ? widget->GetHeight() : size.height;
							size.width = accumulatedPos;
						}

						if (first) first = false;

					});

				SetSize(size);
			}

			void OnSizeChanged(const SizeF&, const SizeF&) override
			{
				UpdateLayout();
			}

		public:
			Stack(bool vertical = true, float borderSize = 2.0f)
				: m_vertical(vertical)
				, m_borderSize(borderSize)
			{
				m_borderSize = 7.0f;
			}

			void Add(std::unique_ptr<Widget> widget)
			{
				AddChild(std::move(widget));

				UpdateLayout();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin)
				{
					context.skin->DrawStack(*this, context);
				}
			}
		};

		class UniformGrid : public Widget
		{
		private:
			engine::container::Grid<Widget*> m_cells;
			float m_borderSize;

		protected:
			void OnSizeChanged(const SizeF&, const SizeF&) override
			{
				UpdateLayout();
			}

			void UpdateCellLayout(int row, int col, Widget* widget)
			{
				if (!widget) return;

				SizeF cellsize = GetCellSize();

				PositionF pos
				{
					col * cellsize.width + m_borderSize * col,
					row * cellsize.height + m_borderSize * row
				};

				widget->SetPosition(pos);
				widget->SetSize(cellsize);
			}

			void UpdateLayout()
			{
				if (m_cells.GetWidth() == 0 || m_cells.GetHeight() == 0)
				{
					throw std::runtime_error("why is size < 1? this is impossible");
				}

				for (int row = 0; row < m_cells.GetHeight(); ++row)
				{
					for (int col = 0; col < m_cells.GetWidth(); ++col)
					{
						Widget* widget = Get(row, col);

						UpdateCellLayout(row, col, widget);
					}
				}
			}

			virtual void OnSet(int row, int col, Widget* widget)
			{
			}

		public:
			UniformGrid(size_t rows = 1, size_t cols = 1, float borderSize = 2.0f)
				: m_borderSize(borderSize)
			{
				SetGridSize(rows, cols);

				m_moveBehavior = MoveBehavior::None;
				m_droppable = false;
				m_focusable = false;
			}

			void SetGridSize(size_t rows, size_t cols)
			{
				// brute force for now. just clear the grid before resizing
				RemoveChildren();
				m_cells.Clear();

				// clamp to 1. min size is always 1x1
				if (rows < 1) rows = 1;
				if (cols < 1) cols = 1;

				m_cells.SetWidth(cols);
				m_cells.Reserve({ cols, rows });

				for (size_t i = 0; i < rows * cols; ++i)
				{
					m_cells.Add(nullptr);
				}

				UpdateLayout();
			}

			Size<size_t> GetGridSize() const
			{
				return m_cells.GetSize();
			}

			SizeF GetCellSize() const
			{
				return SizeF
				{
					(GetWidth() - m_borderSize * (m_cells.GetWidth() - 1)) / static_cast<float>(m_cells.GetWidth()),
					(GetHeight() - m_borderSize * (m_cells.GetHeight() - 1)) / static_cast<float>(m_cells.GetHeight())
				};
			}

			float GetBorderSize() const
			{
				return m_borderSize;
			}

			Widget* Get(int row, int col) const
			{
				return m_cells.Get(row, col);
			}

			void Set(int row, int col, std::unique_ptr<Widget> widget)
			{
				if (!m_cells.IsInBounds(row, col))
				{
					throw std::runtime_error("out of bounds when setting widget");
				}

				Widget* curr = m_cells.Get(row, col);
				if (curr != nullptr)
				{
					RemoveChild(curr);
					m_cells.Set(row, col, nullptr);
				}

				Widget* ptr = widget.get();

				AddChild(std::move(widget));

				m_cells.Set(row, col, ptr);

				UpdateCellLayout(row, col, ptr);

				OnSet(row, col, ptr);

				//ptr->OnMove += [&](const PositionF& pos)
				//	{
				//		// TODO: this is overkill. we just need to update this widget, not the whole grid
				//		//UpdateLayout();
				//	};

				//ptr->OnResize += [&](const SizeF& size)
				//	{
				//		// TODO: this is overkill. we just need to update this widget, not the whole grid
				//		//UpdateLayout();
				//	};
			}

			void Remove(int row, int col)
			{
				if (!m_cells.IsInBounds(row, col))
				{
					throw std::runtime_error("out of bounds when setting widget");
				}

				Widget* widget = Get(row, col);

				if (!widget) return;

				RemoveChild(widget);

				m_cells.Set(row, col, nullptr);
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin)
				{
					context.skin->DrawUniformGrid(*this, context);
				}
			}
		};

		// this widget is used to display a single line of text, just like a label but is focusable, clickable, selectable like a button.
		// it is used as a single item in widgets like TextList 
		class TextItem : public Widget
		{
		protected:
			// reference to the label widget that displays the text. this will be this widget's child
			Label* m_label;

			// when this widget resizes, we need to resize the label to fill the whole area of this widget
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				m_label->SetPosition({ 0.0f, 0.0f });
				m_label->SetSize(newSize);
			}

		public:
			TextItem(const std::string& text)
				: m_label(nullptr)
			{
				// create a label widget to display the text. this label will be a child of this widget
				std::unique_ptr<Label> label = std::make_unique<Label>(text);
				m_label = label.get();
				AddChild(std::move(label));

				// this widget is focusable and selectable like a button, but it does not move like a button. it is just a static text item that can be selected
				m_focusable = true;
				m_moveBehavior = MoveBehavior::None;
			}

			// set the text
			void Set(const std::string& text)
			{
				m_label->Set(text);
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin)
				{
					// TODO: we don't have implementation for drawing TextItem yet...
					// context.skin->DrawTextItem(*this, context);
				}
			}
		};

		// This is a model interface for a list of text items. 
		// It defines the basic operations that any text list model should support, such as getting the size of the list, retrieving an item by index, clearing the list, and adding new items. 
		// It also includes an event that is triggered whenever the model changes, allowing any observers (like a UI component) to react to changes in the data.
		class ITextListModel
		{
		public:
			virtual ~ITextListModel() = default;

			// returns the number of text in the list
			virtual size_t Size() const = 0;

			// returns the text at the given index
			virtual const std::string& Get(size_t index) const = 0;

			// clears the list of text
			virtual void Clear() = 0;

			// event that is triggered whenever the model changes (e.g., when an item is added or removed)
			engine::event::Event<> Changed;

			// remove methods
			virtual void RemoveLast() = 0;
			virtual void RemoveFirst() = 0;
			virtual void RemoveAt(size_t index) = 0;

			// append methods
			virtual void Append(const std::vector<std::string>& texts) = 0;
			virtual void Append(const std::string& text) = 0;

			// insert
			virtual void Insert(size_t index, const std::string& text) = 0;

			// set
			virtual void Set(size_t index, const std::string& text) = 0;
		};

		// This is a concrete implementation of the ITextListModel interface that uses a std::vector to store the list of text items.
		class VectorTextListModel : public ITextListModel
		{
		private:
			std::vector<std::string> m_items;

		public:
			// returns the number of text in the list
			size_t Size() const override
			{
				return m_items.size();
			}

			// returns the text at the given index
			const std::string& Get(size_t index) const override
			{
				return m_items[index];
			}

			// removes the text at the given index and triggers the Changed event
			void RemoveAt(size_t index) override
			{
				assert(index < m_items.size());

				m_items.erase(m_items.begin() + index);
				Changed();
			}

			// inserts text before the given index and triggers the Changed event
			void Insert(size_t index, const std::string& text) override
			{
				index = std::min<size_t>(index, m_items.size());

				m_items.insert(m_items.begin() + index, text);
				Changed();
			}

			// appends multiple texts to the end of the list and triggers the Changed event
			void Append(const std::vector<std::string>& texts) override
			{
				if (texts.empty())
				{
					return;
				}

				m_items.insert(m_items.end(), texts.begin(), texts.end());
				Changed();
			}

			// appends text to the end of the list and triggers the Changed event
			void Append(const std::string& text) override
			{
				m_items.push_back(text);
				Changed();
			}

			// replaces the text at the given index and triggers the Changed event
			void Set(size_t index, const std::string& text) override
			{
				assert(index < m_items.size());

				m_items[index] = text;
				Changed();
			}

			// clears the list of text and triggers the Changed event
			void Clear() override
			{
				if (m_items.empty())
				{
					return;
				}

				m_items.clear();
				Changed();
			}

			// remove the last text in the list and triggers the Changed event
			void RemoveLast() override
			{
				if (m_items.empty())
				{
					return;
				}

				m_items.pop_back();
				Changed();
			}

			// remove the first text in the list and triggers the Changed event
			void RemoveFirst() override
			{
				if (m_items.empty())
				{
					return;
				}

				m_items.erase(m_items.begin());
				Changed();
			}
		};

		// This is a widget that displays a list of text items.
		// It can contain as many text items as needed, but only a subset of them are realized (i.e., created and displayed) based on the current min/max range of visible items.
		// The widget uses a text list model (ITextListModel) to manage the underlying data, and it can be bound to any implementation of that interface.
		class TextList : public Widget
		{
		private:
			// collection of TextItem widgets that are currently realized (i.e., created and displayed) based on the current min/max range of visible items.
			std::vector<TextItem*> m_realizedItems;

			// height of each item in the list. this is used to calculate the position of each item based on its index in the list.
			float m_itemHeight;

			// default internal text list model that is used if no external model is provided. this allows the TextList to manage its own data if needed.
			VectorTextListModel m_internalTextListModel;

			// pointer to the current text list model that is being used by the TextList. this can be either the internal model or an external model provided by the user.
			ITextListModel* m_textListModel;

			// the current minimum and maximum indices of the visible items in the list. these are used to determine which items should be realized and displayed.
			int m_min;
			int m_max;

		protected:
			// update the layout of the realized items when the size of the TextList changes.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				UpdateLayout();
			}

			// synchronizes the size of realized items with the number of visible items based on the current min/max range.
			// this does not refresh the text or position of the realized items, it only ensures that the number of realized items matches the number of visible items.
			void UpdateRealizedItems()
			{
				// current min/max might be out of bounds of the list model. we need to clamp them to valid range
				int size = static_cast<int>(m_textListModel->Size());
				int max = std::min<int>(m_max, size - 1);
				int min = std::min<int>(m_min, size);

				// calculate number of visible items
				int numVisible = max - min + 1;
				numVisible = numVisible < 0 ? 0 : numVisible;

				// do we have more visible labels than supposed to?
				while (m_realizedItems.size() > numVisible)
				{
					RemoveChild(m_realizedItems.back());
					m_realizedItems.pop_back();
				}

				// or is our visible labels not enough?
				while (m_realizedItems.size() < numVisible)
				{
					std::unique_ptr<TextItem> textItem = std::make_unique<TextItem>("");
					m_realizedItems.push_back(textItem.get());
					AddChild(std::move(textItem));
				}
			}

			// updates the layout of the realized items, including their position, size, and text
			void UpdateLayout()
			{
				// first let's update the list of realized items to match the number of visible items
				UpdateRealizedItems();

				// now let's set the text for the visible items
				int currItem = 0;
				for (int i = m_min; i <= m_max; i++)
				{
					if (i >= m_textListModel->Size()) break;

					// be assertive. if we are here, then we must have a realized item for this visible item
					assert(currItem < m_realizedItems.size());

					// set the text for this visible item
					m_realizedItems[currItem]->Set(m_textListModel->Get(i));

					// since we are here, we might as well set this visible label's position
					m_realizedItems[currItem]->SetPosition(
						{
							0.0f,
							m_itemHeight * i
						});

					// we set the size as well
					m_realizedItems[currItem]->SetSize(
						{
							GetSize().width,
							m_itemHeight
						});

					// move to next visible item
					currItem++;
				}

				// and finally we set the size of this widget. 
				// this forces the this widget to have a height that can accommodate all the items in the list, even if they are not all visible at once.
				SetSize(
					{
						GetSize().width,
						m_itemHeight * m_textListModel->Size()
					});
			}

			// handler for when the text list model changes. this will trigger an update of the layout to reflect the changes in the underlying data.
			void OnChangeTextListModel()
			{
				UpdateLayout();

				// bubble up the event to notify that the text list has changed. this allows any observers (like a UI component) to react to changes in the data.
				Changed();
			}

		public:

			// event that is triggered whenever the text list changes (e.g., when a text is added or removed)
			engine::event::Event<> Changed;

			TextList(float itemHeight = 40.0f)
				: m_itemHeight(itemHeight)
				, m_min(0)
				, m_max(-1)
				, m_textListModel(nullptr)
			{
				// it's ok for this widget to be movable. on its own it cannot be dragged because it resizes to fit its content which are the realized items. 
				// realized items are TextItem widgets and they are not movable
				m_moveBehavior = MoveBehavior::Free;

				// by default, we will use the internal text list model. this allows the TextList to manage its own data if needed.
				m_textListModel = &m_internalTextListModel;
				m_textListModel->Changed += engine::event::Handler(this, &TextList::OnChangeTextListModel);
			}

			~TextList()
			{
				// unsubscribe from the text list model's OnChanged event to avoid dangling references and potential crashes when the TextList is destroyed.
				if (m_textListModel)
				{
					m_textListModel->Changed -= engine::event::Handler(this, &TextList::OnChangeTextListModel);
				}
			}

			// bind this TextList to a new text list model. if no model is provided, it will use the internal model.
			void Bind(ITextListModel* model = nullptr)
			{
				m_textListModel->Changed -= engine::event::Handler(this, &TextList::OnChangeTextListModel);

				m_textListModel = model ? model : &m_internalTextListModel;

				m_textListModel->Changed += engine::event::Handler(this, &TextList::OnChangeTextListModel);

				// when we bind to a new model, we need to update the layout to reflect the changes in the underlying data.
				UpdateLayout();
			}

			// remove all items and clear the list
			void Clear()
			{
				// set min/max to default values. this will ensure that no items are visible and the realized items will be cleared.
				m_min = 0;
				m_max = -1;

				// remove all items from the list and clear the realized items as well as TextList is subscribed to the OnChanged event of the model 
				// which will call UpdateLayout() to clear the realized items and reset the size of the TextList to accommodate the empty list.
				m_textListModel->Clear();
			}

			void SetItemHeight(float itemHeight)
			{
				m_itemHeight = itemHeight;
				UpdateLayout();
			}

			float GetItemHeight() const
			{
				return m_itemHeight;
			}

			void SetMin(int min)
			{
				m_min = min;
				UpdateLayout();
			}

			void SetMax(int max)
			{
				m_max = max;
				UpdateLayout();
			}

			void Append(const std::string& text)
			{
				// since we are subscribed to the OnChanged event of the model, adding an item will automatically trigger an update of the layout to reflect the changes in the underlying data.
				m_textListModel->Append(text);
			}

			void RemoveLast()
			{
				m_textListModel->RemoveLast();
			}

			void RemoveFirst()
			{
				m_textListModel->RemoveFirst();
			}

			void RemoveAt(size_t index)
			{
				m_textListModel->RemoveAt(index);
			}

			void Insert(size_t index, const std::string& text)
			{
				m_textListModel->Insert(index, text);
			}

			void Set(size_t index, const std::string& text)
			{
				m_textListModel->Set(index, text);
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin)
				{
					context.skin->DrawTextList(*this, context);
				}
			}
		};

		class TextListBox : public Widget
		{
		protected:
			ScrollView* m_scrollView;
			TextList* m_textList;

			// handle size changes of this widget.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				// update the layout of all widgets dependent on this widget's size. 
				UpdateLayout();
			}

			// updates the extents and positions of child widgets 
			void UpdateLayout()
			{
				// the child scrollview will occupy the whole area of this widget so we always resize it to match our size

				// it's guaranteed by design that we have a scrollview child. so we can safely assume m_scrollView is not null
				m_scrollView->SetPosition({ 0.0f, 0.0f });
				m_scrollView->SetSize(GetSize());
			}

		public:
			TextListBox()
				: m_scrollView(nullptr)
				, m_textList(nullptr)
			{
				// create a scroll view and add it as a child
				std::unique_ptr<ScrollView> scrollview = std::make_unique<ScrollView>();
				scrollview->SetPosition({ 0, 0 });
				m_scrollView = scrollview.get();
				AddChild(std::move(scrollview));

				// subscribe to scrollview's viewport resize event. 
				m_scrollView->ViewPortResized += [&](const SizeF& size)
					{
						// when scrollview's size change, adjust content's width to match scrollview's width. height will be determined by content's own size
						m_textList->SetSize({ size.width, m_textList->GetSize().height });

						// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
						float textListItemHeight = m_textList->GetItemHeight();
						VecF offset = m_scrollView->GetViewPortOffset();
						int min = static_cast<int>(offset.y / textListItemHeight);
						int max = static_cast<int>((offset.y + m_scrollView->GetViewPortSize().height) / textListItemHeight);

						m_textList->SetMin(min);
						m_textList->SetMax(max);
					};

				// subscribe to scrollview's content change event
				m_scrollView->ContentChanged += [&](Widget* content)
					{
						// when scrollview's content changes, adjust content's width to match scrollview's width. height will be determined by content's own size
						content->SetSize({ m_scrollView->GetViewPortSize().width, content->GetSize().height });

						// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
						float textListItemHeight = m_textList->GetItemHeight();
						VecF offset = m_scrollView->GetViewPortOffset();
						int min = static_cast<int>(offset.y / textListItemHeight);
						int max = static_cast<int>((offset.y + m_scrollView->GetViewPortSize().height) / textListItemHeight);

						m_textList->SetMin(min);
						m_textList->SetMax(max);
					};

				// subscribe to scrollview's scroll event. 
				// when scrollview's content moves or scrolls, we need to update our textlist's min/max visible items based on the new viewport offset and item height
				m_scrollView->Scroll += [&](const VecF& offset)
					{
						// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
						float textListItemHeight = m_textList->GetItemHeight();
						int min = static_cast<int>(offset.y / textListItemHeight);
						int max = static_cast<int>((offset.y + m_scrollView->GetViewPortSize().height) / textListItemHeight);

						m_textList->SetMin(min);
						m_textList->SetMax(max);
					};

				// create a textlist and add it as the content of the scrollview and add it as content of the scrollview. 
				// this will allow the textlist to be scrolled within the scrollview.
				std::unique_ptr<TextList> textList = std::make_unique<TextList>();
				textList->SetPosition({ 0, 0 });
				m_textList = textList.get();
				m_scrollView->SetContent(std::move(textList));

				// subscribe to textlist's change event. when a text is added or removed, 
				// we need to recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
				m_textList->Changed += [&]()
					{
						// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
						float textListItemHeight = m_textList->GetItemHeight();
						VecF offset = m_scrollView->GetViewPortOffset();
						int min = static_cast<int>(offset.y / textListItemHeight);
						int max = static_cast<int>((offset.y + m_scrollView->GetViewPortSize().height) / textListItemHeight);

						m_textList->SetMin(min);
						m_textList->SetMax(max);
					};
			}

			void Append(const std::string& text)
			{
				// add to TextList. 
				m_textList->Append(text);
			}

			void RemoveLast()
			{
				m_textList->RemoveLast();
			}

			void Draw(const UIDrawContext& context) const override
			{
				if (context.skin)
				{
					//	context.skin->DrawTextListBox(*this, context);
				}
			}

			void Clear()
			{
				m_textList->Clear();
			}
		};
#pragma endregion

#pragma region // UI theme/skin

		class DefaultUISkin : public UISkin
		{
		public:
			void DrawButton(const Button& button, const UIDrawContext& context) const override
			{
				PositionF pos = button.GetAbsolutePosition();
				SizeF size = button.GetSize();

				if (&button == context.capture)
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				//else if (&button == context.focus)
				//{
				//	context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				//	context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				//	context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0,0,0,1 }, 0);
				//	context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				//}
				else if (&button == context.hover)
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.55f,0.55f,0.55f,1 }, 0);
				}
				else
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
			}

			void DrawDraggable(const Draggable& button, const UIDrawContext& context) const override
			{
				PositionF pos = button.GetAbsolutePosition();
				SizeF size = button.GetSize();

				if (&button == context.capture)
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				else if (&button == context.hover)
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.55f,0.55f,0.55f,1 }, 0);
				}
				else
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
			}

			void DrawLayer(const class Layer& overlay, const UIDrawContext& context) const override
			{
				PositionF pos = overlay.GetAbsolutePosition();
				SizeF size = overlay.GetSize();

				context.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				ColorF color = (&overlay == context.focus) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);

				if (overlay.IsMenu())
				{
					PositionF ownerPos = overlay.GetOwner()->GetAbsolutePosition();
					SizeF ownerSize = overlay.GetOwner()->GetSize();

					context.renderer.Draw(pos + PositionF{ 1, 0 }, SizeF{ ownerSize.width - 2, 1 }, color, 0);
				}
				else if (overlay.GetType() == Layer::SubMenu)
				{
					SizeF ownerSize = overlay.GetOwner()->GetSize();

					context.renderer.Draw(pos + PositionF{ 0, 1 }, SizeF{ 1, ownerSize.height - 2 }, color, 0);
				}
			}

			void DrawTooltip(const class Tooltip& tooltip, const UIDrawContext& context) const override
			{
				PositionF pos = tooltip.GetAbsolutePosition();
				SizeF size = tooltip.GetSize();

				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 1,1,1,1 }, 0);
			}

			void DrawLabel(const class Label& label, const UIDrawContext& context) const override
			{
				IFontAtlas* font = context.system.GetFont(label.GetFontType());
				if (!font)
				{
					throw std::runtime_error("font does not exist");
				}

				PositionF pos = label.GetTextAbsolutePosition();
				if (label.GetParent() && label.GetParent() == context.capture) pos += PositionF{ 1, 1 };

				ColorF color = { 0.3f,0.3f,0.3f,1 };
				if (label.GetParent() && label.GetParent() == context.focus) color = { 0, 0, 0, 1 };

				context.renderer.Draw(*font, label.Get(), pos, color);
			}

			void DrawImage(const class Image& image, const UIDrawContext& ctx) const override
			{
				Sprite sprite = image.Get();

				PositionF pivot = sprite.GetPivot();

				PositionF pivotInPixels = image.IsStretched() ? PositionF{ pivot.x * image.GetWidth(), pivot.y * image.GetHeight() } : sprite.GetPivotInPixels();

				PositionF pos = (image.IsStretched() ? image.GetAbsolutePosition() : image.GetImageAbsolutePosition()) + pivotInPixels;

				SizeF size = image.IsStretched() ? image.GetSize() : sprite.GetSize();

				ctx.renderer.Draw(image.GetImageAbsolutePosition(), image.IsStretched() ? image.GetSize() : sprite.GetSize(), { 0,1,0,0.5f }, 0);
				ctx.renderer.Draw(image.GetAbsolutePosition(), image.GetSize(), { 0,0,0,0.5f }, 0);

				ctx.renderer.Draw(sprite, pos, size, { 1,1,1,1 }, 0);
			}

			void DrawFrame(const class Frame& frame, const UIDrawContext& ctx) const override
			{
				PositionF pos = frame.GetAbsolutePosition();
				SizeF size = frame.GetSize();

				ctx.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

				ctx.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				ColorF color = (&frame == ctx.focus) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
				ctx.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
			}

			void DrawMenuButton(const MenuButton& menuButton, const UIDrawContext& context) const override
			{
				PositionF pos = menuButton.GetAbsolutePosition();
				SizeF size = menuButton.GetSize();
				bool isExpanded = context.system.IsLayerExpanded(&menuButton);

				if (&menuButton == context.capture)
				{
					ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

					if (isExpanded)
					{
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, color, 0);
					}
					else
					{
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
					}
				}
				else if (&menuButton == context.hover)
				{
					ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

					if (isExpanded)
					{
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, { 0.5f,0.5f,0.5f,1 }, 0);
					}
					else
					{
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
					}
				}
				else
				{
					if (isExpanded)
					{
						context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, { 0.5f,0.5f,0.5f,1 }, 0);
					}
				}
			}

			void DrawSubMenuButton(const SubMenuButton& subMenuButton, const UIDrawContext& context) const override
			{
				PositionF pos = subMenuButton.GetAbsolutePosition();
				SizeF size = subMenuButton.GetSize();
				bool isExpanded = context.system.IsLayerExpanded(&subMenuButton);
				ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };


				if (&subMenuButton == context.capture || &subMenuButton == context.hover)
				{
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - (isExpanded ? SizeF{ 1,2 } : SizeF{ 2,2 }), color, 0);
				}
				else
				{
					if (isExpanded)
					{
						context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
						context.renderer.Draw(pos + PositionF{ 1, 1 }, size - (isExpanded ? SizeF{ 1,2 } : SizeF{ 2,2 }), color, 0);
					}
				}
			}

			void DrawMenuItem(const MenuItem& menuItem, const UIDrawContext& context) const override
			{
				PositionF pos = menuItem.GetAbsolutePosition();
				SizeF size = menuItem.GetSize();

				if (&menuItem == context.capture)
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				else if (&menuItem == context.hover)
				{
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.6f,0.6f,0.6f,1 }, 0);
				}
				else
				{
				}
			}

			void DrawSlider(const Slider& slider, const UIDrawContext& context) const override
			{
				PositionF pos = slider.GetAbsolutePosition();
				SizeF size = slider.GetSize();

				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}

			void DrawThumb(const Thumb& thumb, const UIDrawContext& context) const override
			{
				PositionF pos = thumb.GetAbsolutePosition();
				SizeF size = thumb.GetSize();

				ColorF color = (&thumb == context.hover) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };

				context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 6,6 }, { 0.5f,0.5f,0.5f,1 }, 0);


			}

			void DrawCheckBox(const CheckBox& checkbox, const UIDrawContext& context) const override
			{

			}

			void DrawRadioButton(const RadioButton& radiobutton, const UIDrawContext& context) const override
			{
				PositionF pos = radiobutton.GetAbsolutePosition();
				SizeF size = radiobutton.GetSize();

				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);

				if (radiobutton.IsOn())
				{
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 6, 6 }, { 0,0,0,1 }, 0);
				}
			}

			void DrawScrollBar(const ScrollBar& scrollbar, const UIDrawContext& context) const override
			{
				PositionF pos = scrollbar.GetAbsolutePosition();
				SizeF size = scrollbar.GetSize();

				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}

			void DrawGrip(const Grip& grip, const UIDrawContext& context) const override
			{
				PositionF pos = grip.GetAbsolutePosition();
				SizeF size = grip.GetSize();

				ColorF color = (&grip == context.hover) ? ColorF{ 0.5f,0,0,0.4f } : ColorF{ 0.5f,0,0,0.2f };
				context.renderer.Draw(pos, size, color, 0);

			}

			void DrawResizeableFrame(const ResizeableFrame& frame, const UIDrawContext& context) const override
			{
				PositionF pos = frame.GetAbsolutePosition();
				SizeF size = frame.GetSize();

				//context.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

				//context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				//ColorF color = (&frame == context.focus) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
				//context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);

				context.renderer.Draw(pos, size, { 0,0.5f,0,0.5f }, 0);

			}

			void DrawViewPort(const ViewPort& vp, const UIDrawContext& context) const override
			{
				PositionF pos = vp.GetAbsolutePosition();
				SizeF size = vp.GetSize();

				context.renderer.Draw(pos, size, { 0,0,1,0.3f }, 0);
			}

			void DrawContent(const Content& content, const UIDrawContext& context) const override
			{
				PositionF pos = content.GetAbsolutePosition();
				SizeF size = content.GetSize();

				context.renderer.Draw(pos, size, { 0,0.5f,0,0.3f }, 0);
			}

			void DrawScrollView(const ScrollView& scrollview, const UIDrawContext& context) const override
			{
				PositionF pos = scrollview.GetAbsolutePosition();
				SizeF size = scrollview.GetSize();

				context.renderer.Draw(pos, size, { 1,0,0,0.3f }, 0);
			}

			void DrawUniformGrid(const UniformGrid& grid, const UIDrawContext& context) const override
			{
				PositionF pos = grid.GetAbsolutePosition();
				SizeF size = grid.GetSize();

				Size<size_t> gridSize = grid.GetGridSize();
				float borderSize = grid.GetBorderSize();
				SizeF cellSize = grid.GetCellSize();

				for (int row = 0; row < gridSize.height; row++)
				{
					PositionF cellPos = pos;
					cellPos.y += (row * cellSize.height + borderSize * row);

					for (int col = 0; col < gridSize.width; col++)
					{
						cellPos.x = pos.x + (col * cellSize.width + borderSize * col);

						context.renderer.Draw(cellPos, cellSize, { 0.5f, 0.5f,0.5f, 1 }, 0.0f);
					}
				}
			}

			void DrawStack(const Stack& stack, const UIDrawContext& context) const override
			{
				PositionF pos = stack.GetAbsolutePosition();
				SizeF size = stack.GetSize();

				context.renderer.Draw(pos, size, { 0.1f, 0.1f,0.1f, 1 }, 0.0f);
			}

			void DrawTextListBox(const TextListBox& box, const UIDrawContext& context) const override
			{
				PositionF pos = box.GetAbsolutePosition();
				SizeF size = box.GetSize();

				context.renderer.Draw(pos, size, { 1,0,0,0.3f }, 0);
			}

			void DrawTextList(const TextList& text, const UIDrawContext& context) const override
			{
				PositionF pos = text.GetAbsolutePosition();
				SizeF size = text.GetSize();

				context.renderer.Draw(pos, size, { 1,0,1,0.3f }, 0);
			}

		};

#pragma endregion

	}
}