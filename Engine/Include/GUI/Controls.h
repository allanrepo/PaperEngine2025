#pragma once
#include <GUI/Widget.h>
#include <GUI/UISystem.h>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/Animated.h>
#include <Graphics/Core/Renderable.h>
#include <Graphics/Core/Sprite.h>
#include <algorithm>

namespace engine
{
	namespace gui
	{
#pragma region // 
		using IRenderable = engine::graphics::IRenderable;
		using Sprite = engine::graphics::Sprite;
		using Renderable = engine::graphics::Renderable;
		using Animated = engine::graphics::Animated;
#pragma endregion

#pragma region // Label
		// non interactive widget that displays text using sytem fonts
		class Label : public Widget
		{
		private:
			// text to be displayed
			std::string m_text;

			// font type.
			UIResources::FontType m_fontType;

			// text alignment inside this widget's layout. 
			// an alignment with both horizontal and vertical set to center means the text will be rendered at the center of the widget
			VerticalAlignment m_vAlign;
			HorizontalAlignment m_hAlign;

			// size of the text with respect to the current font
			SizeF m_textSize;

			// top-left position of the text relative to widget's layout
			PositionF m_textPosition;

		protected:
			// this is fired up when this widget is added to a widget tree with a UI system. 
			bool OnRegisterToSystem() override final;

			// this is fired up when this widget is removed from a widget tree with a UI system. 
			bool OnUnregisterToSystem() override final;

			// when font resource change in UI system this widget is registered to, this callback is called
			void OnResourceChange() override final;

			// when this widget is resized, text position must be updated
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final;

			// calculates the position of the text given the font and alignment
			bool UpdateLayout();

		public:
			// constructor
			Label(const std::string& text, UIResources::FontType fontType = UIResources::FontType::Default);

			// non-copyable, non-movable
			Label(const Label&) = delete;
			Label& operator=(const Label&) = delete;
			Label(Label&&) noexcept = delete;
			Label& operator=(Label&&) noexcept = delete;

			// absolute position of text's top-left corner
			PositionF GetTextAbsolutePosition() const;

			// set font type
			void SetFontType(UIResources::FontType type);

			// get font type
			UIResources::FontType GetFontType() const;

			// get text
			std::string Get() const;

			// set text
			void Set(const std::string& text);

			// set alignment
			void SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign);

			// draw
			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // Image
		// non interactive widget used to display renderable objects
		class Image : public Widget
		{
		private:
			// renderable resource. this can be a Renderable or Animated
			// only reason why we store it as unique_ptr is so it can be polymorphic
			// other than that, IRenderables are lightweight views that can be copied around.
			std::unique_ptr<IRenderable> m_image;

			// image alignment inside this widget's layout. 
			// an alignment with both horizontal and vertical set to center means the image will be rendered at the center of the widget
			VerticalAlignment m_vAlign;
			HorizontalAlignment m_hAlign;

			// top-left position of the image relative to widget's layout
			PositionF m_imagePosition;

			// if true, the image will be stretched to fill the widget's layout
			bool m_stretch = false;

		protected:
			// this is fired up when this widget is added to a widget tree with a UI system. 
			bool OnRegisterToSystem() override final;

			// reposition the image based on its alignment
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final;

			// calculates the position of the image based on alignment
			bool UpdateLayout();

		public:
			// constructor for Renderable as image resource
			Image(Renderable renderable);

			// constructor for Animated as image resource
			Image(Animated animated);

			// non-copyable, non-movable
			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;
			Image(Image&&) noexcept = delete;
			Image& operator=(Image&&) noexcept = delete;

			void EnableStretch(bool stretch);

			bool IsStretched() const;

			// absolute position of image's top-left corner
			PositionF GetImageAbsolutePosition() const;

			// get sprite 
			Sprite Get() const;

			// draw
			void Draw(const UIDrawContext& context) const override;

			// set alignment
			void SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign);
		};
#pragma endregion

#pragma region // Frame
		class Frame : public Widget
		{
		private:

		public:
			Frame(bool movable = true, bool droppable = true, bool focusable = false, bool interactive = true);

			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // Button
		class Button : public Widget
		{
		private:
		public:
			Button();

			void Draw(const UIDrawContext& context) const override;

			event::Event<> Click;

		protected:
			void OnMouseUp(const PositionF& position) override;

		};
#pragma endregion

#pragma region // Thumb
		class Thumb : public Widget
		{
		private:
			friend class ScrollBar;
			friend class Slider;

		public:
			Thumb();
			Thumb(bool isHorizontal);

			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // Slider
		class Slider : public Widget
		{
		private:
			float m_min;
			float m_max;
			float m_value;

			bool m_horizontal;
			Thumb* m_thumb;
			float m_thumbLength;
			bool m_isDragging;
			int m_steps;

		protected:
			// Begins dragging the thumb and updates the value based on the mouse position.
			virtual void OnMouseDown(const PositionF& position);

			// Stops dragging the thumb.
			void OnMouseUp(const PositionF& position) override;

			// Updates the value while the thumb is being dragged.
			void OnMouseMove(const PositionF& position) override;

			// Updates the thumb size and position when the slider is resized.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			// Updates the thumb position to reflect the current value.
			void UpdateThumbPosition();

			// Updates the thumb size based on the slider's orientation and thumb length.
			void UpdateThumbSize();
			
			// Updates the slider's value based on the thumb's position.
			void UpdateValueFromPosition(const PositionF& position);

			// Snaps the given value to the nearest step based on the slider's step size or count.
			float Snap(float value) const;

		public:
			// Event that is fired when the slider's value changes. The new value is passed as a float parameter.
			engine::event::Event<float> Change;

			// Creates a slider with the specified value range and thumb length.
			// @param min The minimum value of the slider.
			// @param max The maximum value of the slider.
			// @param thumbLength The length of the thumb along the slider's direction.
			Slider(float min, float max, float thumbLength);

			// Sets whether the slider is horizontal or vertical.
			void Horizontal(bool enable);

			// Returns true if the slider is horizontal, false if it is vertical.
			bool Horizontal() const;

			// Sets the length of the thumb along the slider's direction.
			void SetThumbLength(float length);

			// Sets the minimum value of the slider.
			void Min(float min);

			// Sets the maximum value of the slider.
			void Max(float max);

			// Returns the minimum value of the slider.
			float Min() const;

			// Returns the maximum value of the slider.
			float Max() const;

			// Sets the current value of the slider.
			void Value(float value);

			// Returns the current value of the slider.
			float Value() const;

			// Sets the step size of the slider.
			void SetStepSize(float size);

			// Sets the number of steps of the slider.
			void SetStepCount(int count);

			// Draws the slider using the provided draw context.
			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // ScrollBar
		// design principle:
		// - thumb position and movement is always decided by logical position. only then it gets translated into physical position
		// - offset must be able to set between 0 to max boundary or logical range, even if scrolling is discrete. 

		// provides a visual and interactive mechanism for navigating content that is larger than its viewport.
		// behavior is determined by three primary values:
		//	- Content length — the length of the content being viewed.
		//	- Viewport length — the length of the visible area.
		//	- Scroll offset — the current logical position within the scrollable content.
		// 
		// Scrollbar and Thumb geometry:
		//  - scrollbar has a physical length, which determines the available space in which the thumb can move. it represents the content's length
		//  - thumb's length represents the visible area of the content. it represents the viewport's length
		//  - when scrolling is possible, the thumb's physical length is calculated proportionally
		//		- thumbLength = scrollbarLength * viewportLength / contentLength;
		// 
		// illustration of a scrollbar with physical length of 100.
		// - content length is 4 and viewport length is 3. 
		// - thumb represents the viewport hence the calculated thumb length is 75 (scrollbar length * viewport length / content length)
		// - track length is the distance thumb can travel within the scrollbar. in our case, it is 25
		// - this means that thumb's position (left position in our example) relative to scrollbar ranges from 0 to 25
		// 
		//		Scrollbar (length = 100)
		//		0                                                                   100
		//		|-------------------------------------------------------------------- - |
		//		|===============================================|-------------------- - |
		//		0                                              75                    100
		//		|<--------------Thumb = 75 -------------------> | < --Track = 25 -----> |
		// 
		// resolving thumb's physical position based on logical offset:
		// - Although the thumb can be physically moved between positions 0 and 25, its final position will always be realigned to match the logical offset.
		// - The thumb represents the viewport. In our example, the viewport can only occupy 2 logical positions: 0 and 1.
		//		- At logical position 0, the viewport displays content from 1 to 3.
		//		- At logical position 1, the viewport displays content from 2 to 4.
		// - Although the thumb can physically move anywhere between positions 0 and 25, its final position will always be realigned to match the viewport's logical position. 
		// - In our example, the thumb can therefore end up only at position 0 or position 25.
		// 
		// use case: when viewport is larger than content, or viewport size is 0, or content size is 0:
		// - in either of these cases, no scrolling is possible
		// - in this case, thumb fills the entire scrollbar and will be positioned at 0,0 of scrollbar
		// - because thumb fills that entire scrollbar, it cannot be dragged.
		// - the logical scroll offset is 0
		// 
		// thumb's minimum physical length:
		//	- athumb that becomes too small can be difficult to see and interact with.
		//  - to address this, the scrollbar defines a minimum thumb length. 
		//  - the thumb must never exceed the scrollbar's physical length. 
		//	- when the minimum thumb length exceeds the calculated length, the thumb no longer represents the exact visible proportion of the content. 
		// 
		// use case: when scrollbar's physical length becomes too small and thumb ends up filling the scrollbar
		//	- assuming current logical offset value in our example is 1 and scrollbar has been resized to be too small and forced thumb to fill the scrollbar
		//	- thumb will now be repositioned at 0,0. this does not change the logical offset. logical offset remains 1.
		class ScrollBar : public Widget
		{
		public:
			enum class ScrollMode
			{
				Smooth,
				Discrete
			};

		private:
			float m_contentLength;
			float m_viewportLength;
			float m_offset; // current scroll position
			bool m_horizontal;
			Thumb* m_thumb;
			bool m_isDragging;
			float m_minThumbLength;

			bool m_ignoreThumbMove = false;

			ScrollMode m_scrollMode = ScrollMode::Discrete;
			float m_stepSize = 1.5f;

		protected:
			// checks if the scrollbar can scroll. if content is smaller than viewport, it cannot scroll
			bool CanScroll() const;

			// callback when mouse is pressed down on the scroll bar. this can happen on the thumb or on the track. 
			// if it is on the track, we will move the thumb to that position and start dragging. 
			// if it is on the thumb, we will start dragging the thumb.
			void OnMouseDown(const PositionF& pos) override;

			// callback when mouse is released on the scroll bar. this can happen on the thumb or on the track.
			void OnMouseUp(const PositionF&) override;

			// callback when mouse is moved on the scroll bar. this can happen on the thumb or on the track.
			void OnMouseMove(const PositionF& pos) override;

			// internal method to update the size of the thumb based on the content length, viewport length, and scrollbar length. this is called when content or viewport changes.
			void UpdateThumbSize();

			float LogicalToPhysicalPosition(float logicalPos) const;
			float PhysicalToLogicalPosition(float physicalPos) const;

			float SnapToNearestStep(float value, float step) const;

			// callback when the size of the scrollbar changes. this is called when the scrollbar is resized. we will update the thumb size and position based on new size
			void OnSizeChanged(const SizeF&, const SizeF&) override;

		public:
			// event that is raised when internal offset changed. offset is the logical position of the thumb
			// logical position is the position of viewport along the content
			engine::event::Event<float> Scroll;

			// constructor
			ScrollBar(float contentLength, float viewportLength, bool isHorizontal, float minThumLength = 16.0f, float stepSize = 1.0f, ScrollBar::ScrollMode mode = ScrollBar::ScrollMode::Discrete);

			// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset is changed.
			void SetOffset(float offset);

			// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
			float GetOffset() const;

			// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
			void SetContentLength(float length);

			// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
			void SetViewportLength(float length);

			void SetStepSize(float step);
			float GetStepSize() const;

			void SetScrollMode(ScrollMode mode);
			ScrollMode GetScrollMode() const;

			float GetContentLength() const;
			float GetViewportLength() const;

			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // ResizeableFrame
		// a frame the can be resized when dragging its edge/corner grips
		// it has a min size that clamps to it when resizing the frame via grips
		// TODO:
		// originally, content component is a Content widget. this is because we want ResizeableFrame to be draggable when clicked and drag at its inner
		// area, where the content widget is. but ResizeableFrame should not be draggable. it is only resizeable. so we remove that feature.
		// now we don't need content component to be Content widget. the handles for content widget's drag behavior is removed.
		// but for now it is just commented out, not totally removed because we want to implement a widget later that can be used as anchor to drag
		// another widget. we plan to implement a dialog that can be both resized and dragged. and we want it to have an anchor component that when
		// clicked and dragged, will move the dialog along with it.
		class ResizeableFrame : public Widget
		{
		public:
			class Content : public Widget
			{
			private:
				friend class ResizeableFrame;

			public:
				Content()
				{
					m_moveBehavior = MoveBehavior::None;
					m_droppable = false;
					m_focusable = false;
				}
			};

			class Grip : public Widget
			{
			private:
				friend class ResizeableFrame;

			public:
				Grip(bool MoveHorizontal, bool MoveVertical);
			};

			// resize grip components
			Grip* m_leftResizeGrip = nullptr;
			Grip* m_rightResizeGrip = nullptr;
			Grip* m_topResizeGrip = nullptr;
			Grip* m_bottomResizeGrip = nullptr;
			Grip* m_topLeftResizeGrip = nullptr;
			Grip* m_topRightResizeGrip = nullptr;
			Grip* m_bottomLeftResizeGrip = nullptr;
			Grip* m_bottomRightResizeGrip = nullptr;

			// content widget that is the child of this frame. it is the area that is inside the frame's border
			// this is the area that is resized when the frame is resized. it is also the area that is used to layout the child widgets of this frame
			// this is not a draggable widget. it is not used to drag the frame.
			Widget* m_content = nullptr;

			// resize grip thickness
			float m_borderSize;

			// min size when resizing through grips
			SizeF m_minResize;

			// resizing trackers
			PositionF m_beginPosition;
			SizeF m_beginSize;

		protected:
			// callback when this widget's size has changed. its layout must be updated.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			// internal method to update its layout. all its grips must be resized and reposition. content widget must be resized to fit the inner area 
			void UpdateLayout();

			// update content widget by refreshing its position and resizing to fit the inner area 
			virtual void UpdateContentLayout();

			// update grip to reposition and resize when widget has resized
			void UpdateTopRightGripLayout();
			void UpdateTopGripLayout();
			void UpdateRightGripLayout();
			void UpdateTopLeftGripLayout();
			void UpdateLeftGripLayout();
			void UpdateBottomRightGripLayout();
			void UpdateBottomGripLayout();
			void UpdateBottomLeftGripLayout();

			// clamp the size of widget to minimum allowed size if the given size is smaller than that
			SizeF ClampSize(const SizeF& size) const;

		public:
			// constructor
			ResizeableFrame(float borderSize = 8.0f, const SizeF& minSize = { 100, 100 });

			// set clamp size
			void SetMinResize(const SizeF& size);

			// set thickness of the grips
			void SetBorderSize(float size);

			// event that is invoked when its content widget has been resized
			engine::event::Event<const SizeF&> ContentAreaSizeChanged;

			// draw
			void Draw(const UIDrawContext& context) const override;

			// add widget into content area
			void AddContent(std::unique_ptr<Widget> widget);

			// getters
			SizeF GetContentsize() const;
			SizeF GetContentAreaSize() const;
		};

		class Dialog
			: public ResizeableFrame
		{
		protected:
			float m_titleBarHeight;
			bool m_titleBarEnabled;

			Widget* m_titleBar = nullptr;

			void UpdateContentLayout() override final
			{
				m_content->SetPosition({ m_borderSize, m_borderSize + (m_titleBarEnabled? m_titleBarHeight: 0.0f)});

				float width = GetSize().width - m_borderSize * 2;
				if (width < 0) width = 0.0f;

				float height = GetSize().height - m_borderSize * 2 - (m_titleBarEnabled ? m_titleBarHeight : 0.0f);
				if (height < 0) height = 0.0f;

				m_content->SetSize({ width, height });
			}

		public:
			Dialog(
				float borderSize = 8.0f,
				const SizeF& minSize = { 200, 200 },
				float titleBarHeight = 32.0f,
				bool titleBarEnabled = true
			)
				: m_titleBarHeight(titleBarHeight)
				, m_titleBarEnabled(titleBarEnabled)
				, ResizeableFrame(borderSize, minSize)
			{
				// make it movable
				m_moveBehavior = MoveBehavior::Free;

			}

			// draw
			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawResizeableFrame(*this, context);
			}

		};

#pragma endregion

#pragma region // Panel - non drawable, non focusable, none droppable, non movable widget designed to hold child widgets and manage its layout in group.
		class Panel : public Widget
		{
		private:

		public:
			Panel(bool interactive = true);
		};
#pragma endregion

#pragma region // DragHandle - handle its drag state and delegate the drag movement to its referenced proxy widget
		// this is a behavioral or interaction widget.
		// it has position and size, receives and responds to input, interprets input and translates to drag/movement. that makes it a widget.
		// it is not visual, hence no draw handling required.
		// its purpose is to handle its drag state and delegate the drag movement to its referenced proxy widget
		// although its move behavior is that it's movable, it does not really move. it only delegates move/drag state to its referenced proxy widget
		// it's named DragHandle because it is a handle that can be used to drag its referenced proxy widget around. it is not a visual handle, but a behavioral handle.
		// it's analogous to a door knob that can be used to open a door. the knob itself does not open the door, but it is a handle that can be used to open the door. 
		class DragHandle : public Widget
		{
		protected:
			Widget& m_widget;
			bool m_isMoving = false;
			PositionF m_beginMovePosition;

		public:
			DragHandle(Widget& widget, bool moveHorizontal = true, bool moveVertical = true);
		};
#pragma endregion

#pragma region // ScrollView
		// ScrollView manages the overall scrolling control. 
		// It arranges the viewport and scrollbars, determines which scrollbars are needed, 
		// and synchronizes their sizes and positions with the content.
		class ScrollView : public Widget
		{
		public:
#pragma region // ViewPort - container widget that holds a single "content" widget and manages the content widget's scrolling.
			// ViewPort manages the content being viewed.It owns the content widget, moves it to implement scrolling, 
			// keeps its position within valid bounds, and publishes events when the content moves, resizes, or changes.
			// ViewPort doesn't need to know how content moves. It simply reports that its content moved or changed size
			// if content is larger than viewport, it ensures that content always fill the whole region of the viewport.
			// if content is smaller than viewport, it ensures the content stays put and its top-left is at 0,0 of viewport
			// NOTE: inner class because i think this is ScrollView's implementation detail.
			class ViewPort : public Widget
			{
			protected:
				Widget* m_content;

				// this method ensures that content's position is always within the viewport's bounds. 
				// if content's position is outside the viewport, it will be moved back to the nearest position within the viewport
				bool UpdateContentPosition();

				// handler for when viewport's size changes. we need to ensure that content's position is still within the viewport's bounds
				void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

				// event handler for when content's size changes. we need to ensure that content's position is still within the viewport's bounds
				void OnContentSizeChanged(const SizeF& size);

				// callback when content change position. 
				void OnContentMove(const PositionF& pos);

			public:
				// event invoked when content changes position
				engine::event::Event<const VecF&> Scroll;

				// event invoked when content's size changed
				engine::event::Event<const SizeF&> ContentSizeChanged;

				// event invoked when content object changed
				engine::event::Event<Widget*> ContentChanged;

				ViewPort();

				void SetContent(std::unique_ptr<Widget> content);

				void SetContentSize(const SizeF& size);

				SizeF GetContentSize() const;

				VecF GetOffset() const;

				void SetOffset(const VecF& offset);

				void AddContent(std::unique_ptr<Widget> content);
			};
#pragma endregion

		private:
			bool m_ignoreScrollBarScroll = false;
			bool m_ignoreViewPortScroll = false;

		protected:
			// components
			ViewPort* m_viewport;
			ScrollBar* m_hScrollBar;
			ScrollBar* m_vScrollBar;

			// if true, scrollbars are hidden if not needed
			bool m_autoHideScrollBars;

			float m_scrollSize;
			float m_borderSize;

		protected:

			// this will update thumb positions of the scroll bars
			void UpdateThumbPositions();

			// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarContentSize();

			// this will update the viewport size of scroll bars.  scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarViewportSize();

			void UpdateLayout();

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			void UpdateComponentLayout();

		public:

			engine::event::Event<const VecF&> Scroll;
			engine::event::Event<Widget*> ContentChanged;
			engine::event::Event<const SizeF&> ViewPortResized;

			ScrollView(bool autoHideScrollBars = true, float scrollSize = 20.0f, float borderSize = 2.0f);

			void SetContentSize(const SizeF& size);

			void SetContent(std::unique_ptr<Widget> content);

			// should this be called View? or Client? what's the best name for this?
			SizeF GetViewPortSize() const;

			VecF GetViewPortOffset() const;

			void SetContentPosition(const PositionF& pos);

			void Draw(const UIDrawContext& context) const override;

		};
#pragma endregion
	}
}