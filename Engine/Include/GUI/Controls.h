#pragma once
#include <GUI/Widget.h>
#include <GUI/UISystem.h>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/Animated.h>
#include <Graphics/Core/Renderable.h>
#include <Graphics/Core/Sprite.h>
#include <algorithm>
#include <Graphics/Core/Color.h>

namespace engine
{
	namespace gui
	{
#pragma region // Namespaces
		using IRenderable = engine::graphics::IRenderable;
		using Sprite = engine::graphics::Sprite;
		using Renderable = engine::graphics::Renderable;
		using Animated = engine::graphics::Animated;
		using ColorF = engine::graphics::ColorF;
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

			ColorF m_color;

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

			// set color
			void SetColor(const ColorF& color);

			// get font type
			UIResources::FontType GetFontType() const;
			
			// get color
			ColorF GetColor() const;

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
		// description:
		//	- non interactive widget used to display renderable objects
		// design considerations:
		//	- why do we care about image's sprite pivot? 
		//		- because renderer will take sprite's pivot into account.
		//		- it will translate sprite's top-left position with its given pivot.
		//		- pivot is part of sprite's geometric description. Renderer must always be aware of it and take it into account when rendering sprite
		//	- why is IRenderable image object reference a unique_ptr? Isn't IRenderable a lightweight view object only?
		//		- only reason why we store it as unique_ptr is so it can be polymorphic. other than that, IRenderables are lightweight views that can be copied around.
		class Image : public Widget
		{
		private:
			// renderable resource. this can be a Renderable or Animated
			// only reason why we store it as unique_ptr is so it can be polymorphic
			// other than that, IRenderables are lightweight views that can be copied around.
			std::unique_ptr<IRenderable> m_image;

			// image alignment inside this widget's layout. 
			VerticalAnchor m_vImageAnchor;
			HorizontalAnchor m_hImageAnchor;

		protected:
		public:
			// constructor for Renderable as image resource
			Image(Renderable renderable);

			// constructor for Animated as image resource
			Image(Animated animated);

			// set renderable resource 
			void Set(Renderable renderable);
			void Set(Animated animated);

			// non-copyable, non-movable
			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;
			Image(Image&&) noexcept = delete;
			Image& operator=(Image&&) noexcept = delete;

			// get the image's absolute position, accounting for image's sprite pivot 
			PositionF GetImageAbsolutePosition() const;

			// get the image's position relative to the widget's coordinates, accountin image's sprite pivot 
			PositionF GetImagePosition() const;

			// get the image size. this will return image size based on image anchor type
			SizeF GetImageSize() const;

			// get sprite 
			Sprite Get() const;

			// draw
			void Draw(const UIDrawContext& context) const override;

			// set alignment
			void SetHorizontalImageAnchor(HorizontalAnchor hAnchor);
			void SetVerticalImageAnchor(VerticalAnchor vAnchor);
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
			Thumb(engine::gui::Orientation isHorizontal = engine::gui::Orientation::Horizontal);

			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // Slider
		class Slider : public Widget
		{
		private:
			class SliderThumb : public Thumb
			{
				public:
				SliderThumb(engine::gui::Orientation isHorizontal = engine::gui::Orientation::Horizontal)
					: Thumb(isHorizontal)
				{
				}

				void Draw(const UIDrawContext& context) const override
				{
					context.skin.DrawSliderThumb(*this, context);
				}
			};

			float m_min;
			float m_max;
			float m_value;

			Orientation m_orientation;
			SliderThumb* m_thumb;
			float m_thumbLength;
			bool m_isDragging;
			int m_range;

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
			Slider(float min, float max, float thumbLength, int range, Orientation orientation = Orientation::Horizontal);

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

			// Sets the number of steps of the slider.
			void SetRange(int range);

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
			class ScrollThumb : public Thumb
			{
			public:
				ScrollThumb(engine::gui::Orientation isHorizontal = engine::gui::Orientation::Horizontal)
					: Thumb(isHorizontal)
				{
				}

				void Draw(const UIDrawContext& context) const override
				{
					context.skin.DrawScrollThumb(*this, context);
				}
			};

			float m_contentLength;
			float m_viewportLength;
			float m_offset; // current scroll position
			Thumb* m_thumb;
			bool m_isDragging;
			float m_minThumbLength;

			bool m_ignoreThumbMove = false;

			ScrollMode m_scrollMode = ScrollMode::Discrete;
			float m_stepSize = 1.5f;

			engine::gui::Orientation m_orientation = engine::gui::Orientation::Horizontal;

			float m_mouseWheelScrollFactor = 0.2f;

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
			ScrollBar(
				float contentLength, 
				float viewportLength, 
				engine::gui::Orientation orientation, 
				float minThumLength = 16.0f, 
				float stepSize = 1.0f, 
				ScrollBar::ScrollMode mode = ScrollBar::ScrollMode::Discrete
			);

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
		// design principle:
		// - has 3 components - viewport, horizontal scrollbar, vertical scrollbar
		// - ScrollView is the main orchestrator of the scrolling behavior. viewport and scrollbars don't know about each other.
		// - their events are handled by ScrollView to synchronize their behavior.
		// - when resize happen, ScrollView fully manages the layout and behavior of its components to ensure all of them are in sync.
		// - when scrollbars are scrolled, ScrollView handles its events and update the viewport's content position accordingly.
		// - when viewport's content is scrolled, ScrollView handles its events and update the scrollbars' thumb position accordingly.
		class ScrollView : public Widget
		{
		public:
#pragma region // ViewPort - container widget that holds a single "content" widget and manages the content widget's scrolling.
			// design principle:
			// - Purpose
			//		- it's a container of a single widget. this can be any type of widget. it's called content
			//		- it controls the position of the content widget. 
			//			- it makes sure it's top-left position is 0,0 or less
			//			- and if it's top-left position is 0,0 or less, it's bottom-right position is widget's width/height or more
			//			- it basically forces content widget to always fill this viewport's extent
			//			- and if the content's size is smaller than viewport extent, it's locked into 0,0 position
			// - Setting content
			//		- when setting content, it updates content's position relative to its new parent - ViewPort.
			//		- this is because it's possible the content's position/size before it become's this ViewPort's content is out of ViewPort's bounds
			//		- ContentChange event is also invoked so listeners are notified for content change
			//		- if content's position is changed by viewport, content invokes Moved event which viewport listens to
			//			- viewport's content "Moved" event handler will again update content's position. 
			//			- but since content is now within the bounds of the viewport, it is guaranteed to never set position again. 
			//			- that ensures no stack overflow from recursion
			//			- the Moved event handler will then raise "Scroll" event
			//				- the scroll event will notify listeners that the position of the content relative to viewport changed
			//				- this is useful when a listener like scrollbar needs to update its thumb position relative to content's position
			//				- it will then update it by the given "offset"
			// - what is "offset"?
			//		- it is the distance between viewport's origin (0,0) and content's top-left position (origin - position)
			//		- since content's top-left position cannot be greater than 0,0 this value is 0,0 or +x, +y
			// 
			// NOTE: inner class because i think this is ScrollView's implementation detail.
			class ViewPort : public Widget
			{
			protected:
				// reference to the content widget. it does not own it. it is just controlled by ViewPort.
				Widget* m_content;

				// this method ensures that content's position is always within the viewport's bounds. 
				// if content's position is outside the viewport, it will be moved back to the nearest position within the viewport
				bool ResolveContentPosition();

				// handler for when viewport's size changes. we need to ensure that content's position is still within the viewport's bounds
				void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

				// callback when content's size changes. we need to ensure that content's position is still within the viewport's bounds
				void OnContentSizeChanged(const SizeF& oldSize, const SizeF& newSize);

				// callback when content change position. 
				void OnContentMove(const PositionF& oldPos, const PositionF& newPos);


			public:
				// event invoked when content changes position
				engine::event::Event<const VecF&> Scroll;

				// event invoked when content's size changed
				engine::event::Event<const SizeF&> ContentSizeChanged;

				// event invoked when content object changed
				engine::event::Event<Widget*> ContentChanged;

				// constructor
				ViewPort();

				// set the content widget. this will replace the current content widget if any. it will also set the content's position to 0,0 of viewport
				void SetContent(std::unique_ptr<Widget> content);

				// set the content size. this will resize the content widget to the given size. it will also ensure that content's position is still within the viewport's bounds
				void SetContentSize(const SizeF& size);

				// set the content's position. this will move the content widget to the given position. it will also ensure that content's position is still within the viewport's bounds
				void SetOffset(const VecF& offset);

				// getters
				SizeF GetContentSize() const;
				VecF GetOffset() const;
				Widget* GetContent() const;
			};
#pragma endregion

		private:
			bool m_ignoreScrollBarScroll = false;
			bool m_ignoreViewPortScroll = false;

			float m_mouseWheelScrollFactor = 0.2f;

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

			void OnVerbose(bool enable) override;

			// this will update thumb positions of the scroll bars
			void UpdateScrollBarOffsets();

			// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarContentSize();

			// this will update the viewport size of scroll bars.  scroll bars then will refresh its thumb size and thumb position internally
			void UpdateScrollBarViewportSize();

			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			void UpdateComponentLayout();

			bool OnMouseWheel(const PositionF& screenPos, float delta) override;
			bool OnMouseHWheel(const PositionF& screenPos, float delta) override;

		public:

			engine::event::Event<const VecF&> Scroll;
			engine::event::Event<Widget*> ContentChanged;
			engine::event::Event<const SizeF&, const SizeF&> ViewPortResized;

			ScrollView(bool autoHideScrollBars = true, float scrollSize = 20.0f, float borderSize = 2.0f);

			void SetContentSize(const SizeF& size);

			void SetContent(std::unique_ptr<Widget> content);

			// should this be called View? or Client? what's the best name for this?
			SizeF GetViewPortSize() const;

			VecF GetViewPortOffset() const;

			void SetOffset(const VecF& offset);

			void SetContentPosition(const PositionF& pos);

			void Draw(const UIDrawContext& context) const override;

		};
#pragma endregion
	}
}

namespace engine
{
	namespace gui
	{
#pragma region // Namespaces
		//using IRenderable = engine::graphics::IRenderable;
		//using Sprite = engine::graphics::Sprite;
		//using Renderable = engine::graphics::Renderable;
		//using Animated = engine::graphics::Animated;
		//using ColorF = engine::graphics::ColorF;
#pragma endregion

#pragma region // TextItem
		// this widget is used to display a single line of text, just like a label but is focusable, clickable, selectable like a button.
		// it is used as a single item in widgets like TextList 
		class TextItem : public Widget
		{
		protected:
			// reference to the label widget that displays the text. this will be this widget's child
			Label* m_label;

			bool m_selected = false;

			// when this widget resizes, we need to resize the label to fill the whole area of this widget
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				m_label->SetPosition({ 0.0f, 0.0f });
				m_label->SetSize(newSize);
			}

			// raise click event when this item is selected
			void OnMouseDown(const PositionF& pos) override
			{
				Clicked(this, m_label->Get());
			}

		public:
			event::Event<TextItem*, const std::string&> Clicked;

			TextItem(const std::string& text, UIResources::FontType type = UIResources::FontType::Default)
				: m_label(nullptr)
				, m_selected(false)
			{
				// create a label widget to display the text. this label will be a child of this widget
				std::unique_ptr<Label> label = std::make_unique<Label>(text, type);
				m_label = label.get();
				AddChild(std::move(label));

				// this widget is focusable and selectable like a button, but it does not move like a button. it is just a static text item that can be selected
				m_focusable = true;
				m_moveBehavior = MoveBehavior::None;
			}

			void SetFontType(UIResources::FontType type)
			{
				m_label->SetFontType(type);
			}

			UIResources::FontType GetFontType() const
			{
				return m_label->GetFontType();
			}

			// set the text
			void Set(const std::string& text)
			{
				m_label->Set(text);
			}

			std::string Get() const
			{
				return m_label->Get();
			}

			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawTextItem(*this, context);
			}

			void SetSelected(bool selected)
			{
				m_selected = selected;
			}

			void Select()
			{
				m_selected = true;
			}

			void Unselect()
			{
				m_selected = false;
			}

			bool IsSelected() const
			{
				return m_selected;
			}
		};
#pragma endregion

#pragma region // ITextListModel
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
#pragma endregion

#pragma region // VectorTextListModel
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
#pragma endregion

#pragma region // TextList
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

			UIResources::FontType m_fontType = UIResources::FontType::Default;

			// flags used to manage layout updating 
			int m_updating = 0;
			bool m_dirty = false;

			std::unordered_set<int> m_selectedIndices;
			SelectionMode m_selectionMode = SelectionMode::Multiple;
			bool m_allSelected = false;

		protected:
			// update the layout of the realized items when the size of the TextList changes.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
			{
				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
			}

			void OnClickTextItem(TextItem* item, const std::string& text)
			{
				// do a linear search to find which item is the clicked one. this is fine. 
				// i think realized items list will never be too big that doing this will cause performance issue
				auto it = std::find(
					m_realizedItems.begin(),
					m_realizedItems.end(),
					item);

				if (it == m_realizedItems.end())
					return;

				int realizedIndex = static_cast<int>(std::distance(m_realizedItems.begin(), it));

				int modelIndex = m_min + realizedIndex;

				// resolve selection
				{
					switch (m_selectionMode)
					{
					case SelectionMode::None:
						break;
					case SelectionMode::Single:
					{
						// clear selected indices then insert our newly selected item index
						m_selectedIndices.clear();
						m_selectedIndices.insert(modelIndex);

						// unselect all realized items first. since realized item list is expected to never be big, iterating here is ok
						for (TextItem* item : m_realizedItems)
						{
							item->Unselect();
						}

						// now we set the clicked item as selected
						m_realizedItems[realizedIndex]->Select();

						break;
					}
					case SelectionMode::Multiple:
					{
						auto it = m_selectedIndices.find(modelIndex);
						if (it != m_selectedIndices.end())
						{
							m_selectedIndices.erase(it);
							m_allSelected ? m_realizedItems[realizedIndex]->Select() : m_realizedItems[realizedIndex]->Unselect();

						}
						else
						{
							m_selectedIndices.insert(modelIndex);
							m_allSelected ? m_realizedItems[realizedIndex]->Unselect() : m_realizedItems[realizedIndex]->Select();
						}
						break;
					}
					default:
						break;
					}
				}

				Clicked(modelIndex, text);
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
					m_realizedItems.back()->Clicked -= engine::event::Handler(this, &TextList::OnClickTextItem);
					RemoveChild(m_realizedItems.back());
					m_realizedItems.pop_back();
				}

				// or is our visible labels not enough?
				while (m_realizedItems.size() < numVisible)
				{
					std::unique_ptr<TextItem> textItem = std::make_unique<TextItem>("", m_fontType);
					textItem->SetName("TextList.TextItem");

					m_realizedItems.push_back(textItem.get());

					textItem->Clicked += engine::event::Handler(this, &TextList::OnClickTextItem);

					AddChild(std::move(textItem));
				}
			}

			// updates the layout of the realized items, including their position, size, and text
			void UpdateLayout()
			{
				// if we are currently updating the layout, we don't want to trigger another update. 
				// instead, we mark the layout as dirty and return. this will ensure that the layout is updated after the current update is finished.
				if (m_updating > 0
					|| !m_dirty
					)
				{
					return;
				}

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

					// resolve select state of realized items
					{
						if (m_allSelected)
						{
							m_selectedIndices.find(i) != m_selectedIndices.end() ? m_realizedItems[currItem]->Unselect() : m_realizedItems[currItem]->Select();
						}
						else
						{
							m_selectedIndices.find(i) != m_selectedIndices.end() ? m_realizedItems[currItem]->Select() : m_realizedItems[currItem]->Unselect();
						}
					}

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

				// since we have updated the layout, we can clear the dirty flag
				m_dirty = false;
			}

			// handler for when the text list model changes. this will trigger an update of the layout to reflect the changes in the underlying data.
			void OnChangeTextListModel()
			{
				m_dirty = true;
				if (m_updating == 0) UpdateLayout();

				// bubble up the event to notify that the text list has changed. this allows any observers (like a UI component) to react to changes in the data.
				Changed();
			}

		public:

			engine::event::Event<int, const std::string&> Clicked;

			// event that is triggered whenever the text list changes (e.g., when a text is added or removed)
			engine::event::Event<> Changed;

			TextList(float itemHeight = 40.0f, UIResources::FontType fontType = UIResources::FontType::Default)
				: m_itemHeight(itemHeight)
				, m_min(0)
				, m_max(-1)
				, m_textListModel(nullptr)
				, m_fontType(fontType)
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

				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
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

				UnselectAll();
			}

			void UnselectAll()
			{
				m_allSelected = false;

				for (TextItem* item : m_realizedItems)
				{
					item->Unselect();
				}

				m_selectedIndices.clear();
			}

			void SelectAll()
			{
				if (m_selectionMode != SelectionMode::Multiple) return;

				m_allSelected = true;
				m_selectedIndices.clear();
				for (TextItem* item : m_realizedItems)
				{
					item->Select();
				}
			}

			void SetFontType(UIResources::FontType type)
			{
				if (m_fontType == type) return;

				m_fontType = type;
				for (TextItem* item : m_realizedItems)
				{
					item->SetFontType(type);
				}
			}

			UIResources::FontType GetFontType() const
			{
				return m_fontType;
			}

			void SetItemHeight(float itemHeight)
			{
				if (m_itemHeight == itemHeight) return;
				m_itemHeight = itemHeight;

				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
			}

			void BeginUpdate()
			{
				m_updating++;
			}

			void EndUpdate()
			{
				m_updating--;
				if (m_updating < 0) m_updating = 0;

				if (m_updating == 0) UpdateLayout();
			}

			float GetItemHeight() const
			{
				return m_itemHeight;
			}

			// min can be > max. it just means nothing is visible
			void SetMin(int min)
			{
				if (m_min == min) return;

				m_min = min;

				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
			}

			// max can be < min. it just means nothing is visible
			void SetMax(int max)
			{
				if (m_max == max) return;

				m_max = max;

				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
			}

			// max can be < min. it just means nothing is visible
			// min can be > max. it just means nothing is visible
			void SetRange(int min, int max)
			{
				if (m_min == min && m_max == max) return;

				m_min = min;
				m_max = max;

				m_dirty = true;
				if (m_updating == 0) UpdateLayout();
			}

			int GetMin() const
			{
				return m_min;
			}

			int GetMax() const
			{
				return m_max;
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

			void SetSelectionMode(SelectionMode mode)
			{
				if (m_selectionMode == mode) return;

				m_selectionMode = mode;

				// everytime selection mode changes, just safely clear currently selected items
				UnselectAll();
			}

			SelectionMode GetSelectionMode() const
			{
				return m_selectionMode;
			}

			size_t Size() const
			{
				return m_textListModel->Size();
			}

			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawTextList(*this, context);
			}
		};
#pragma endregion

#pragma region // TextListBox
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

			// updates the layout of the text list based on the current size of the scroll view's viewport.
			// this will adjust the text list's width to match the scroll view's viewport width and 
			// recalculate the min/max visible items based on the new viewport size, item height, and current viewport offset.
			void UpdateTextListLayout()
			{
				// always adjust the textlist's width to match the scrollview's viewport width. height will be determined by content's own size
				m_textList->SetSize({ m_scrollView->GetViewPortSize().width, m_textList->GetSize().height });

				// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
				UpdateTextListMinMax();
			}

			// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
			void UpdateTextListMinMax()
			{
				// find the min/max which are the indices of the text items that are currently visible in the viewport. 
				float textListItemHeight = m_textList->GetItemHeight();
				VecF offset = m_scrollView->GetViewPortOffset();
				int min = static_cast<int>(offset.y / textListItemHeight);
				int max = static_cast<int>((offset.y + m_scrollView->GetViewPortSize().height) / textListItemHeight);

				// then update textlist with new min/max
				m_textList->BeginUpdate();
				m_textList->SetMax(max);
				m_textList->SetMin(min);
				m_textList->EndUpdate();
			}

			void OnVerbose(bool enable)
			{
				m_scrollView->Verbose(enable);
				m_textList->Verbose(enable);
			}

		public:
			TextListBox(float itemHeight = 40.0f, UIResources::FontType fontType = UIResources::FontType::Default)
				: m_scrollView(nullptr)
				, m_textList(nullptr)
			{
				// create a scroll view and add it as a child
				std::unique_ptr<ScrollView> scrollview = std::make_unique<ScrollView>();
				scrollview->SetPosition({ 0, 0 });
				scrollview->SetName("TextListBox.ScrollView");
				m_scrollView = scrollview.get();
				AddChild(std::move(scrollview));

				// subscribe to scrollview's viewport resize event. 
				m_scrollView->ViewPortResized += [&](const SizeF& oldSize, const SizeF& size)
					{
						UpdateTextListLayout();
					};

				// subscribe to scrollview's content change event
				m_scrollView->ContentChanged += [&](Widget* content)
					{
						UpdateTextListLayout();
					};

				// when scrollview's content moves or scrolls, we need to update our textlist's min/max visible items based on the new viewport offset and item height
				m_scrollView->Scroll += [&](const VecF& offset)
					{
						UpdateTextListMinMax();
					};

				// create a textlist and add it as the content of the scrollview and add it as content of the scrollview. 
				// this will allow the textlist to be scrolled within the scrollview.
				// NOTE: we create textlist object and add its content to scrollview here AFTER we subscribe to scrollview's events so we trigger ScrollView's ContentChanged event
				std::unique_ptr<TextList> textList = std::make_unique<TextList>(itemHeight, fontType);
				textList->SetPosition({ 0, 0 });
				textList->SetName("TextListBox.TextList");
				m_textList = textList.get();
				m_scrollView->SetContent(std::move(textList));

				// subscribe to textlist's change event. when a text is added or removed, 
				// we need to recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
				m_textList->Changed += [&]()
					{
						UpdateTextListMinMax();
					};

				// handle event when a text item is clicked
				m_textList->Clicked += [&](int index, const std::string& text)
					{
						// if selected item is partially visible, reposition so that it is fully visible
						{
							// scenario > offset: 24, itemHeight = 10, selected index (selIndex) = 2
							//		- selected position (selPos) = index * itemHeight = 20
							//		- if selPos < offset, selected text item is at least partially not visible at the top of viewport. need to adjust
							//			- offset(new) = selPos
							// scenario > offset: 24, itemHeight = 10, viewportSize = 23, selected index (selIndex) = 5
							//		- selected position (selPos) = index * itemHeight = 50
							//		- if selPos + itemHeight > offset + viewportSize, selected text item is at least partially not visiable at the bottom of viewport, need to adjust
							//		- offset(new) = offset + (selPos + itemHeight) - (offset + viewportSize)
							//		- offset(new) = 24 + 60 - 57 = 27
							// scenario > itemHeight > viewportSize
							//		- resolving at bottom of viewport takes precedence

							VecF offset = m_scrollView->GetViewPortOffset();
							float itemHeight = m_textList->GetItemHeight();
							SizeF viewportSize = m_scrollView->GetViewPortSize();
							float selPos = index * itemHeight;

							if (selPos < offset.y)
							{
								offset.y = selPos;
								m_scrollView->SetOffset(offset);
							}

							if (selPos + itemHeight > offset.y + viewportSize.height)
							{
								offset.y += (selPos + itemHeight) - (offset.y + viewportSize.height);

								m_scrollView->SetOffset(offset);
							}
						}

						Clicked(text);
					};
			}

			engine::event::Event<const std::string&> Clicked;

			void Append(const std::string& text)
			{
				m_textList->Append(text);
			}

			void RemoveLast()
			{
				m_textList->RemoveLast();
			}

			void SetItemHeight(float itemHeight)
			{
				m_textList->SetItemHeight(itemHeight);
				UpdateTextListLayout();
			}

			float GetItemHeight() const
			{
				return m_textList->GetItemHeight();
			}

			void Draw(const UIDrawContext& context) const override
			{
				context.skin.DrawTextListBox(*this, context);
			}

			void Clear()
			{
				m_textList->Clear();
			}

			void UnselectAll()
			{
				m_textList->UnselectAll();
			}

			void SelectAll()
			{
				m_textList->SelectAll();
			}

			void SetSelectionMode(SelectionMode mode)
			{
				m_textList->SetSelectionMode(mode);
			}

			SelectionMode GetSelectionMode() const
			{
				return m_textList->GetSelectionMode();
			}

			void SetFontType(UIResources::FontType type)
			{
				m_textList->SetFontType(type);
			}

			UIResources::FontType GetFontType() const
			{
				return m_textList->GetFontType();
			}
		};
#pragma endregion
	}
}
