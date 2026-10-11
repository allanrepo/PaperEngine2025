#pragma once
#include <GUI/Controls.h>
#include <GUI/UISkin.h>
#include <GUI/UIRenderer.h>
#include <GUI/UISystem.h>
#include <GUI/DragDropLayer.h>
#include <GUI/Tooltip.h>
#include <GUI/Layer.h>
#include <GUI/Widget.h>

#include <Graphics/Renderer/IRenderer.h>
#include <Math/Size.h>
#include <Core/Event.h>
#include <Spatial/Position.h>
#include <Math/Rect.h>
#include <Containers/Dictionary.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Core/Font.h>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/Sprite.h>
#include <Graphics/Core/Color.h>
#include <Utilities/Logger.h>
#include <algorithm>
#include <Containers/Grid.h>
#include <set>
#include <Graphics/Core/Renderable.h>
#include <Graphics/Core/Animated.h>
//#include <Engine/Gooey/Gooey.h>


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
		using Font = engine::graphics::Font;
		using IRenderable = engine::graphics::IRenderable;
		using Sprite = engine::graphics::Sprite;
		using ColorF = engine::graphics::ColorF;
		using Renderable = engine::graphics::Renderable;
		using Animated = engine::graphics::Animated;

		template<typename K, typename T>
		using Dictionary = engine::container::Dictionary<K, T>;

		template<typename T>
		using Size = engine::math::Size<T>;

#pragma endregion

#pragma region // DraggableItem
		// a widget that can be dragged from one droppable widget into another
		// it's used for inventory systems, skill bars, customizable menus, etc...
		class DraggableItem : public Draggable
		{
		private:
		protected:
			// this widget is draggable via mouse move so we handle start of dragging through mouse down
			virtual void OnMouseDown(const PositionF& position)
			{
				BeginDrag(position);
			}

			// this widget drops on mouse up
			virtual void OnMouseUp(const PositionF& position)
			{
				EndDrag(position);
			}

		public:
			DraggableItem()
			{
			}

			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawDraggable(*this, context);
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



		class MenuButton : public LayerTrigger
		{
		protected:
			//Layer::BuildDescription m_buildDesc;

			// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
			bool OnUnregisterToSystem() override final
			{
				RemoveLayer();

				return true;
			}

			void OnMouseDown(const PositionF& position) override final
			{
				Toggle();
			}

		public:
			MenuButton(const Layer::BuildDescription& buildDesc) 
				: LayerTrigger(buildDesc)
			{
				m_moveBehavior = MoveBehavior::None;
			}

			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawMenuButton(*this, context);
			}

			bool IsExpanded() const
			{
				return LayerTrigger::IsExpanded();
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
				context.skin.DrawSubMenuButton(*this, context);
			}
		};

		class MenuItem : public Button
		{
		public:
			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawMenuItem(*this, context);
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
				context.skin.DrawCheckBox(*this, context);
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
				context.skin.DrawRadioButton(*this, context);
			}
		};



		class Content : public Widget
		{
		private:
			friend class ResizeableFrame;
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
				context.skin.DrawContent(*this, context);
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
				// Moved is only raised when content's position really changed.
				// Update the viewport's layout to ensure content stays within the bounds of viewport's area.
				// Updating the layout may result in setting content's position again which will result in raising Moved.
				// As Moved will only be raised  when content's position really changed, this will not result in recursive loop
				//
				// also note that since we are monitoring content's move event, we don't need to monitor its drag event as dragging will 
				// also eventually set position of content and will raise Moved
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
					m_content->Resized -= engine::event::Handler(this, &ViewPort::OnContentSizeChanged);
					m_content->Moved -= engine::event::Handler(this, &ViewPort::OnContentMove);

					// this will destroy the content widget and all its children. so beware, this is permanent
					RemoveChild(m_content);
					m_content = nullptr;
				}

				// set new content
				m_content = content.get();
				AddChild(std::move(content));

				// subscribe to new content's events
				m_content->Resized += engine::event::Handler(this, &ViewPort::OnContentSizeChanged);
				m_content->Moved += engine::event::Handler(this, &ViewPort::OnContentMove);

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
				// the content's Moved event. ViewPort listens to that event and
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
				context.skin.DrawViewPort(*this, context);
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
				m_viewport->Resized += [&](const SizeF& size)
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
				context.skin.DrawScrollView(*this, context);
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
				context.skin.DrawStack(*this, context);
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

				//ptr->Moved += [&](const PositionF& pos)
				//	{
				//		// TODO: this is overkill. we just need to update this widget, not the whole grid
				//		//UpdateLayout();
				//	};

				//ptr->Resized += [&](const SizeF& size)
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
				context.skin.DrawUniformGrid(*this, context);
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
				// TODO: we don't have implementation for drawing TextItem yet...
				// context.skin->DrawTextItem(*this, context);
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
				context.skin.DrawTextList(*this, context);
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
					//	context.skin->DrawTextListBox(*this, context);
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

				if (context.system.HasMouseCapture(button))
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
				else if (context.system.IsMouseOver(button))
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

				if (context.system.HasMouseCapture(button))
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				else if (context.system.IsMouseOver(button))
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

				ColorF color = (context.system.HasFocus(overlay)) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
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
				//{
				//	PositionF pos = label.GetAbsolutePosition();
				//	SizeF size = label.GetSize();
				//	context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				//}

				Font font = context.system.GetFont(label.GetFontType());
				if (!font.IsValid())
				{
					throw std::runtime_error("font does not exist");
				}

				PositionF pos = label.GetTextAbsolutePosition();
				if (label.GetParent() && context.system.HasMouseCapture(*label.GetParent())) pos += PositionF{ 1, 1 };

				ColorF color = { 0.3f,0.3f,0.3f,1 };
				if (label.GetParent() && context.system.HasMouseCapture(*label.GetParent())) color = { 0, 0, 0, 1 };

				context.renderer.Draw(font, label.Get(), pos, color);
			}

			void DrawImage(const class Image& image, const UIDrawContext& ctx) const override
			{
				Sprite sprite = image.Get();

				PositionF pivot = sprite.GetPivot();

				PositionF pivotInPixels = image.IsStretched() ? PositionF{ pivot.x * image.GetWidth(), pivot.y * image.GetHeight() } : sprite.GetPivotInPixels();

				PositionF pos = (image.IsStretched() ? image.GetAbsolutePosition() : image.GetImageAbsolutePosition()) + pivotInPixels;

				SizeF size = image.IsStretched() ? image.GetSize() : sprite.GetSize();

				//ctx.renderer.Draw(image.GetImageAbsolutePosition(), image.IsStretched() ? image.GetSize() : sprite.GetSize(), { 0,1,0,0.5f }, 0);
				//ctx.renderer.Draw(image.GetAbsolutePosition(), image.GetSize(), { 0,0,0,0.5f }, 0);

				ctx.renderer.Draw(sprite, pos, size, { 1,1,1,1 }, 0);
			}

			void DrawFrame(const class Frame& frame, const UIDrawContext& ctx) const override
			{
				PositionF pos = frame.GetAbsolutePosition();
				SizeF size = frame.GetSize();

				ctx.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

				ctx.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				ColorF color = (ctx.system.HasFocus(*frame.GetParent())) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
				ctx.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
			}

			void DrawMenuButton(const MenuButton& menuButton, const UIDrawContext& context) const override
			{
				
				PositionF pos = menuButton.GetAbsolutePosition();
				SizeF size = menuButton.GetSize();
				bool isExpanded = menuButton.IsExpanded();

				if (context.system.HasMouseCapture(menuButton))
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
				else if (context.system.IsMouseOver(menuButton))
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
				bool isExpanded = subMenuButton.IsExpanded();
				ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };


				if (context.system.HasMouseCapture(subMenuButton) || context.system.IsMouseOver(subMenuButton))
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

				if (context.system.HasMouseCapture(menuItem))
				{
					context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				else if (context.system.IsMouseOver(menuItem))
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

				ColorF color = (context.system.IsMouseOver(thumb)) ? ColorF{0.6f, 0.6f, 0.6f, 1} : ColorF{0.5f, 0.5f, 0.5f, 1};

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

			//void DrawGrip(const Grip& grip, const UIDrawContext& context) const override
			//{
			//	PositionF pos = grip.GetAbsolutePosition();
			//	SizeF size = grip.GetSize();

			//	ColorF color = (context.system.IsMouseOver(grip)) ? ColorF{0.5f,0,0,0.4f} : ColorF{0.5f,0,0,0.2f};
			//	context.renderer.Draw(pos, size, color, 0);

			//}

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