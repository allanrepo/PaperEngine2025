//#include <Spatial/Position.h>
//#include <Math/Size.h>	
//#include <Core/Event.h>
//#include <Math/Rect.h>
//
//#include <vector>
//#include <memory>
//#include <functional>
//
//namespace engine
//{
//	namespace gui
//	{
//#pragma region // forward declarations
//		using PositionF = spatial::PositionF;
//		using SizeF = math::SizeF;
//		using VecF = math::VecF;
//		using RectF = math::RectF;
//
//		class UISystem;
//		class UIDrawContext;
// #pragma endregion
//
//#pragma region // Widget
//		class Widget
//		{
//		private:
//#pragma region // friends
//			friend class UISystem;
//#pragma endregion
//
//#pragma region // DragHandler - private helper class that manages widget's drag operation
//			friend class DragHandler;
//			class DragHandler
//			{
//			private:
//				// widget dragging trackers
//				PositionF m_beginMousePosition;
//				PositionF m_beginMovePosition;
//				bool m_isMoving = false;
//				Widget* m_widget;
//
//			public:
//				DragHandler(Widget* widget);
//
//				// call when widget is about to be dragged, e.g. OnMouseDown
//				void Begin(const PositionF& position);
//
//				// call when widget is being dragged, e.g. OnMouseMove
//				void Update(const PositionF& position);
//
//				// call when widget ends dragging, e.g. OnMouseUp
//				void End(const PositionF& position);
//
//				// getters
//				bool IsDragging() const;
//				PositionF GetBeginPosition() const;
//			};
//#pragma endregion
//
//#pragma region // system registration - internal methods called when widget is added to or removed from UI tree
//			bool UnregisterToSystem();
//			bool RegisterToSystem();
//#pragma endregion
//
//		protected:
//#pragma region // internal property enums
//			// flags for widget's move behavior. this is used to determine if widget can be moved horizontally, vertically, or both
//			enum MoveBehavior
//			{
//				None = 0,
//				Horizontal = 1 << 0,
//				Vertical = 1 << 1,
//				Free = Horizontal | Vertical,
//			};
//
//			// flags for widget's hit test behavior. this is used to determine how widget responds to hit tests
//			// Normal: widget responds to hit tests normally, i.e. it will be hit if point is inside its bounds
//			// AlwaysPass: widget will always be hit, regardless of point being inside its bounds or not
//			// AlwaysFail: widget will never be hit, regardless of point being inside its bounds or not
//			enum class HitTestBehavior
//			{
//				Normal,
//				AlwaysPass,
//				AlwaysFail
//			};
//#pragma endregion
//
//#pragma region // member variables
//			// tree
//			Widget* m_parent = nullptr;
//			std::vector<std::unique_ptr<Widget>> m_children;
//
//			// transform
//			PositionF m_position;
//			SizeF m_size;
//
//			// states
//			bool m_visible = true;
//			bool m_enabled = true;
//
//			// behavior
//			bool m_focusable = true;
//			bool m_droppable = false;
//			MoveBehavior m_moveBehavior = MoveBehavior::Free;
//			HitTestBehavior m_hitTestBehavior = HitTestBehavior::Normal;
//
//			// tooltip support. 
//			// this is a function pointer that is called when tooltip is requested. 
//			// it takes two parameters: the widget that is requesting the tooltip, and the widget that will be used to display the tooltip. 
//			// derived classes can override this function to provide custom tooltip content
//			std::function<void(Widget&, Widget&)> m_tooltipBuilder;
//
//			// widget dragging tracker
//			DragHandler m_dragHandler;
//#pragma endregion
//
//#pragma region // system reference
//			// get system reference from the root of UI tree this widget belongs to. if this widget is not attached to any tree, it will return nullptr
//			virtual UISystem* GetSystem() const
//			{
//				if (m_parent)
//				{
//					return m_parent->GetSystem();
//				}
//
//				return nullptr;
//			}
//#pragma endregion
//
//#pragma region // hooks/callbacks for all actions that widget performs
//			virtual bool OnRegisterToSystem();
//			virtual bool OnUnregisterToSystem();
//			virtual void OnPositionChanged(const PositionF& oldPos, const PositionF& newPos);
//			virtual void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize);
//			virtual void OnResourceChange();
//			virtual void OnMouseDown(const PositionF& position);
//			virtual void OnMouseUp(const PositionF& position);
//			virtual void OnMouseMove(const PositionF& position);
//			virtual void OnMouseEnter();
//			virtual void OnMouseLeave();
//			virtual void OnKeyDown(int key);
//			virtual void OnKeyUp(int key);
//			virtual void OnGotFocus();
//			virtual void OnLostFocus();
//
//#pragma endregion
//
//#pragma region // internal methods for tree traversal and z order
//			// internal enum for search flags. this is used to determine which widgets to consider when searching for top-most widget at given point
//			enum SearchFlags
//			{
//				Visible = 1 << 0,
//				Enabled = 1 << 1,
//				Focusable = 1 << 2,
//			};
//
//			Widget* FindTopWidgetAt(const PositionF& position, unsigned int flag);
//			Widget* FindTopChildAt(const PositionF& position, int flag);
//			Widget* FindAndResolveZOrderAt(const PositionF& position, int flag);
//			bool IsDescendantOf(const Widget* ancestor) const;
//			void BringChildToFront(Widget* child);
//			void BringToFront();
//#pragma endregion
//
//#pragma region // hit test
//			bool Contains(const PositionF& position) const;
//#pragma endregion
//
//#pragma region // drag event 
//			// event args for drag events. this is used to pass information about the drag operation to the event handlers
//			struct DragEventArgs
//			{
//				PositionF beginPosition;
//				PositionF currentPosition;
//
//				VecF Delta() const
//				{
//					return currentPosition - beginPosition;
//				}
//			};
//
//			// these are fired when widget is being dragged. derived classes can subscribe to these events to react to drag operations
//			engine::event::Event<const DragEventArgs&> DragBegin;
//
//			// these are fired when widget is being dragged. derived classes can subscribe to these events to react to drag operations
//			engine::event::Event<const DragEventArgs&> DragMove;
//
//			// these are fired when widget is being dragged. derived classes can subscribe to these events to react to drag operations
//			engine::event::Event<const DragEventArgs&> DragEnd;
//#pragma endregion
//
//#pragma region // resource management. can be called when a resource the widget depends on has changed. derived classes can override the corresponding callback for resource change
//			void ResourceChange();
//#pragma endregion
//
//#pragma region // input handlers. these are called by the UI system when input events are received.
//			void MouseDown(const PositionF& position);
//			void MouseUp(const PositionF& position);
//			void MouseMove(const PositionF& position);
//			void MouseEnter();
//			void MouseLeave();
//			void KeyDown(int key);
//			void KeyUp(int key);
//#pragma endregion
//
//		public:
//#pragma region // RAII
//			Widget()
//				: m_dragHandler(this)
//			{
//			}
//
//			virtual ~Widget() = default;
//#pragma endregion
//
//#pragma region // parameter that determines the horizontal and vertical alignment of the widget's content within its extent
//			enum class HorizontalAlignment
//			{
//				Left,
//				Right,
//				Center
//			};
//
//			enum class VerticalAlignment
//			{
//				Top,
//				Bottom,
//				Center
//			};
//#pragma endregion
//
//#pragma region // heirarchy management
//			void AddChild(std::unique_ptr<Widget> child);
//			void RemoveChild(Widget* widget);
//			void RemoveChildren();
//			Widget* GetParent() const;
//			bool HasChildren() const;
//
//			// remove a widget in this widget tree. this will traverse through this widget's tree to find the widget
//			// if found, removes it as well as its tree. returns true if successfully found and removed
//			bool Remove(Widget* widget);
//
//			// move a child widget to a new parent. this will remove the child from its current parent and add it to the new parent
//			void MoveChildTo(Widget* child, Widget* newParent);
//#pragma endregion
//
//#pragma region // state management
//			// visible widget is rendered and can receive input events. hidden widget is not rendered and cannot receive input events
//			void Show();
//			void Hide();
//			bool IsVisible() const;
//
//			// enabled widget can receive input events. disabled widget cannot receive input events
//			void Enable();
//			void Disable();
//			bool IsEnabled() const;
//#pragma endregion
//
//#pragma region // behavior properties
//			bool IsFocusable() const;
//			bool IsDroppable() const;
//#pragma endregion
//
//#pragma region // transform and extent properties
//			float GetWidth() const;
//			float GetHeight() const;
//			SizeF GetSize() const;
//			engine::event::Event<const SizeF&> Resized;
//			void SetSize(const SizeF& size);
//
//			PositionF GetAbsolutePosition() const;
//			engine::event::Event<const PositionF&> Moved;
//			void SetPosition(const PositionF& pos);
//			PositionF GetPosition() const;
//			RectF GetAbsoluteRect() const;
//#pragma endregion
//
//#pragma region // tree iteration and traversal
//			// iterate through all children of this widget and call the provided function on each child. the function should take a Widget* as parameter
//			template<typename Func>
//			void ForEachChild(const Func& func)
//			{
//				for (const std::unique_ptr<Widget>& child : m_children)
//				{
//					func(child.get());
//				}
//			}
//
//			// iterate through all children of this widget and call the provided function on each child. the function should take a const Widget* as parameter
//			template<typename Func>
//			void ForEachChild(const Func& func) const
//			{
//				for (const std::unique_ptr<Widget>& child : m_children)
//				{
//					func(child.get());
//				}
//			}
//
//			// iterate through all widgets in this widget's tree including itself and call the provided function on each widget. 
//			// the function must return a bool indicating whether to continue iterating or not. if the function returns false, the iteration will stop.
//			// the function should take a Widget* as parameter
//			template<typename Func>
//			bool ForEachWidget(const Func& func)
//			{
//				if (!func(this)) return false;
//
//				for (const std::unique_ptr<Widget>& child : m_children)
//				{
//					if (!child->ForEachWidget(func)) return false;
//				}
//
//				return true;
//			}
//#pragma endregion
//
//#pragma region // tooltip support
//			bool HasTooltip() const;
//			void BuildTooltip(Widget& tooltip);
//			void SetTooltip(std::function<void(Widget&, Widget&)> builder);
//#pragma endregion
//
//#pragma region // rendering
//			virtual void Draw(const UIDrawContext& context) const
//			{
//				// default implementation does nothing. derived class can override this to draw itself
//			}
//#pragma endregion
//		};
//#pragma endregion
//	}
//}