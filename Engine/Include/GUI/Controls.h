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
			VerticalAnchor m_vAlign;
			HorizontalAnchor m_hAlign;

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
			void SetAlignment(VerticalAnchor vAlign, HorizontalAnchor hAlign);

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
		// - behavior is determined by three primary values:
		//		- Content length — the length of the content being viewed.
		//		- Viewport length — the length of the visible area.
		//		- Scroll offset — the current logical position within the scrollable content.
		// - Scrollbar and Thumb geometry:
		//		- scrollbar has a physical length, which determines the available space in which the thumb can move. it represents the content's length
		//		- thumb's length represents the visible area of the content. it represents the viewport's length
		//		- when scrolling is possible, the thumb's physical length is calculated proportionally
		//			- thumbLength = scrollbarLength * viewportLength / contentLength;
		// - illustration of a scrollbar with physical length of 100.
		//		- content length is 40 and viewport length is 30. 
		//		- thumb represents the viewport hence the calculated thumb length is 75 (scrollbar length * viewport length / content length)
		//		- track length is the distance thumb can travel within the scrollbar. in our case, it is 25
		//		- this means that thumb's position (left position in our example) relative to scrollbar ranges from 0 to 25
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
			// scroll type. 
			enum class ScrollMode
			{
				Smooth,
				Discrete
			};

		private:
			// define our own version of thumb so we can implement our own draw method
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

			// represents the logical range of the scrollbar. viewport represents the thumb length while content represents the scrollbar length
			float m_contentLength;
			float m_viewportLength;

			// logical position of viewport(thumb) within content length
			float m_offset; 

			// reference to thumb widget
			Thumb* m_thumb;

			// min thumb length. if content gets way too big compared to viewport, calculated thumb length can be too small. this caps its size
			float m_minThumbLength;

			// internal flag. is true when thumb is being dragged
			bool m_isDragging;

			// internal flag. when true, thumb's move event is ignored
			bool m_ignoreThumbMove = false;

			// scroll mode
			ScrollMode m_scrollMode = ScrollMode::Discrete;

			// step interval where viewport (thumb) is snapped into when moving. this is logical value (based on logical range)
			float m_stepSize = 1.5f;

			// scrollbar can be horizontal or vertical oriented
			engine::gui::Orientation m_orientation = engine::gui::Orientation::Horizontal;

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

			// internal method to update the size of the thumb based on the content length, viewport length, and scrollbar length. 
			void UpdateThumbSize();

			// internal method to convert logical position to physical position and vice versa
			float LogicalToPhysicalPosition(float logicalPos) const;
			float PhysicalToLogicalPosition(float physicalPos) const;

			// internal method to round off the given value to nearest step size
			float SnapToNearestStep(float value, float step) const;

			// callback when the size of the scrollbar changes. this is called when the scrollbar is resized. we will update the thumb size and position based on new size
			void OnSizeChanged(const SizeF&, const SizeF&) override;

		public:
			// event that is raised when internal offset changed. offset is the logical position of the thumb. logical position is the position of viewport along the content
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
			float GetContentLength() const;

			// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
			void SetViewportLength(float length);
			float GetViewportLength() const;

			// set the step size 
			void SetStepSize(float step);
			float GetStepSize() const;

			// set scroll mode
			void SetScrollMode(ScrollMode mode);
			ScrollMode GetScrollMode() const;

			// draw
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
		// - ScrollView orchestrates mouse wheel. all components propagates mouse wheel event notification to ScrollView. ScrollView scrolls all the components
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
#pragma region // TextItem
		// this widget is used to display a single line of text, just like a label but is focusable, clickable, selectable like a button.
		// it is used as a single item in widgets like TextList 
		class TextItem : public Widget
		{
		protected:
			// reference to the label widget that displays the text. this will be this widget's child
			Label* m_label;

			// flag that says if TextItem is selected or not
			bool m_selected = false;

			// when this widget resizes, we need to resize the label to fill the whole area of this widget
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			// raise click event when this item is selected
			void OnMouseDown(const PositionF& pos) override;

		public:
			// raised when TextItem is clicked. click happens on mouse down
			event::Event<TextItem*, const std::string&> Clicked;

			// constructor
			TextItem(const std::string& text, UIResources::FontType type = UIResources::FontType::Default);

			void SetFontType(UIResources::FontType type);

			UIResources::FontType GetFontType() const;

			// set the text
			void Set(const std::string& text);

			std::string Get() const;

			void Draw(const UIDrawContext& context) const override;

			void SetSelected(bool selected);

			void Select();

			void Unselect();

			bool IsSelected() const;
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
			size_t Size() const override;

			// returns the text at the given index
			const std::string& Get(size_t index) const override;

			// removes the text at the given index and triggers the Changed event
			void RemoveAt(size_t index) override;

			// inserts text before the given index and triggers the Changed event
			void Insert(size_t index, const std::string& text) override;

			// appends multiple texts to the end of the list and triggers the Changed event
			void Append(const std::vector<std::string>& texts) override;

			// appends text to the end of the list and triggers the Changed event
			void Append(const std::string& text) override;

			// replaces the text at the given index and triggers the Changed event
			void Set(size_t index, const std::string& text) override;

			// clears the list of text and triggers the Changed event
			void Clear() override;

			// remove the last text in the list and triggers the Changed event
			void RemoveLast() override;

			// remove the first text in the list and triggers the Changed event
			void RemoveFirst() override;
		};
#pragma endregion

#pragma region // TextList
		// design principle:
		//	- purpose
		//		- widget used to manage which set of text among the given text list (model) is realized (e.g. displayed)
		//		- this is made purposely to be content widget of TextListBox
		// 
		//	- realized items list for scalability
		//		- the TextList can represent a very large sized text list (e.g. 10M+), but it does not have to make them all visible.
		//		- only visible text items are realized. so it only stores a list of TextItems that represents the visible items
		//		- this makes it scalable and performance capable regardless of the size of text list
		//		- realized text items does not have fixed association with any texts. they only represent textss that are currently visible
		//		- so when the range of visible text scrolls (min/max changes), realized text items are updated to match the correct texts
		// 
		//	- select/unselect indice list
		//		- to be scalable, we store only a list of realized items, items that are visible. 
		//		- but we can still select multiple items including not visible ones. we use a set container to store indices of selected items. 
		//		- selecting or unselecting all clears selected/unselected list
		// 
		//	- select all mode
		//		- if we select all items, this can be very expensive. imagine selecting all 10M text items. this will insert 10M indices in indice list. that's not scalable
		//		- to make this work, "select all" mode is introduced. when this is enabled (m_allSelected = true), behavior of indice list is inverted
		//		- in "select all" mode, indice list will contain indices that are NOT selected. so when you select all, indice list will be cleared instead.
		//		- likewise, if you "unselect all", indice list behavior is inverted back to its default behavior. it will be cleared and now, any index it contains are selected.
		//		- what if you are in "select all" mode with 10M text items and you keep selecting close to 10M items, you're indice list will now contain almost 10M...
		//			- this is not an issue. selecting is one by one and won't throttle performance. only adding 10M items at once will. we're not doing that.
		// 
		// - using text model as data source
		//		- this is not a container. the actual text data it represents reside somewhere else. 
		//		- since it does not own the text data, it does not care about performance impact of storing large data set, as it only realizes what's visible
		//		- setting or changing text model unselect all items
		// 
		// - deferred layout update 
		//		- say you have 10M texts, and your visible range is 10 (min = 100, max = 109)
		//		- then you scroll down and now range is still 10 but showing different set of text now (min = 20100, max = 20109).
		//		- you first update max to 20109. now temporarily you have a range of 20,010 (min = 100, max - 20109). 
		//		- when update layout is triggered, it will create 20,010 realized text items. THAT IS EXPENSIVE AND UNECESSARY!
		//		- to prevent this, we introduce a deferred layout. this is how it works
		//			- BeginUpdate()
		//			- SetMin()
		//			- SetMax()
		//			- EndUpdate()
		//		- BeginUpdate() will prevent SetMin() and SetMax() from triggering layout update. layout update only happens on EndUpdate()
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

			// font type
			UIResources::FontType m_fontType = UIResources::FontType::Default;

			// internal flags used to manage layout updating 
			int m_updating = 0;
			bool m_dirty = false;

			// selection mode. if multiple, can select multiple items. if single, only one selection at a time
			SelectionMode m_selectionMode = SelectionMode::Multiple;

			// internal container storing selected (or unselected) indices of the text list 
			std::unordered_set<int> m_selectedIndices;

			// selection state. if true, selected indices contains index of items that are not selected. 
			bool m_allSelected = false;

		protected:
			// callback to update the layout of the realized items when the size of the TextList changes.
			void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override;

			// callback to handle event when a TextItem is clicked
			void OnClickTextItem(TextItem* item, const std::string& text);

			// synchronizes the size of realized items with the number of visible items based on the current min/max range.
			// this does not refresh the text or position of the realized items, it only ensures that the number of realized items matches the number of visible items.
			void UpdateRealizedItems();

			// updates the layout of the realized items, including their position, size, and text
			void UpdateLayout();

			// handler for when the text list model changes. this will trigger an update of the layout to reflect the changes in the underlying data.
			void OnChangeTextListModel();

		public:
			// event raised when one of its TextItem is clicked. 
			engine::event::Event<int, bool, const std::string&> Clicked;

			// event that is triggered whenever the text list changes (e.g., when a text is added or removed)
			engine::event::Event<> Changed;

			// constructor
			TextList(float itemHeight = 40.0f, UIResources::FontType fontType = UIResources::FontType::Default);
			~TextList();

			// bind this TextList to a new text list model. if no model is provided, it will use the internal model.
			void Bind(ITextListModel* model = nullptr);

			bool IsBoundTo(ITextListModel* model) const;

			// remove all items and clear the list
			void Clear();

			// unselect all selected items in list if there's any
			void UnselectAll();

			// select all items in list if there's any
			void SelectAll();

			// set font type
			void SetFontType(UIResources::FontType type);

			// getter for font type
			UIResources::FontType GetFontType() const;

			// set item height
			void SetItemHeight(float itemHeight);

			// getter for item height
			float GetItemHeight() const;

			// in cases where layout update needs to be deferred, call this first. any calls that requires layout update will be scoped between BeginUpdate() and EndUpdate()
			void BeginUpdate();

			// closing scope of deferred layout update. call this once all calls requiring layout update are executed and we're ready to update the layout
			void EndUpdate();

			// min can be > max. it just means nothing is visible
			void SetMin(int min);

			// max can be < min. it just means nothing is visible
			void SetMax(int max);

			// max can be < min. it just means nothing is visible. min can be > max. it just means nothing is visible
			void SetRange(int min, int max);

			// getter for min
			int GetMin() const;

			// getter for max
			int GetMax() const;

			// add text at the bottom of the list
			void Append(const std::string& text);

			void RemoveLast();

			void RemoveFirst();

			void RemoveAt(size_t index);

			void Insert(size_t index, const std::string& text);

			void Set(size_t index, const std::string& text);

			// set selection mode. everytime selection mode changes, all selected items are unselected for safety purposes
			void SetSelectionMode(SelectionMode mode);

			SelectionMode GetSelectionMode() const;

			size_t Size() const;

			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // TextListBox
		// design principle:
		//	- purpose
		//		- widget that uses ScrollView to display TextList as its content
		//	
		//	- clicked TextItem will always be fully visible
		//		- if a TextItem is partially visible e.g. top or bottom TextItem in visible range and is clicked, content will scroll so that clicked item becomes fully visible
		//		- if viewport size < Text Item height, and an item is clicked, alignment at bottom of the text item takes precedence
		class TextListBox : public Widget
		{
		protected:
			ScrollView* m_scrollView;
			TextList* m_textList;

			// updates the layout of the text list based on the current size of the scroll view's viewport.
			// this will adjust the text list's width to match the scroll view's viewport width and 
			// recalculate the min/max visible items based on the new viewport size, item height, and current viewport offset.
			void UpdateTextListLayout();

			// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
			void UpdateTextListMinMax();

			void OnVerbose(bool enable);

		public:
			TextListBox(float itemHeight = 40.0f, UIResources::FontType fontType = UIResources::FontType::Default);

			// raise this event when Text Item is clicked and its selected state is toggled
			engine::event::Event<int, bool, const std::string&> Clicked;

			void Append(const std::string& text);

			void RemoveLast();

			void SetItemHeight(float itemHeight);

			float GetItemHeight() const;

			void Draw(const UIDrawContext& context) const override;

			void Clear();

			void UnselectAll();

			void SelectAll();

			void SetSelectionMode(SelectionMode mode);

			SelectionMode GetSelectionMode() const;

			void SetFontType(UIResources::FontType type);

			UIResources::FontType GetFontType() const;

			// bind this TextList to a new text list model. if no model is provided, it will use the internal model.
			void Bind(ITextListModel* model = nullptr);

			bool IsBoundTo(ITextListModel* model) const;
		};
#pragma endregion
	}
}
