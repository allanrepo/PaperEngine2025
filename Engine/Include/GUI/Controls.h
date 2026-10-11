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
			bool RefreshLayout();

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
			bool RefreshLayout();

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
		class ScrollBar : public Widget
		{
		private:
			float m_contentLength;
			float m_viewportLength;
			float m_offset; // current scroll position
			bool m_horizontal;
			Thumb* m_thumb;
			bool m_isDragging;
			float m_minThumbLength;

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

			// internal method to update the position of the thumb when offset, content length, or viewport length changes.
			void UpdateThumbPosition();

			// callback when the size of the scrollbar changes. this is called when the scrollbar is resized. we will update the thumb size and position based on new size
			void OnSizeChanged(const SizeF&, const SizeF&) override;

		public:
			// event that is fired when the scrollbar is scrolled. it will pass the new offset value to the subscribers. 
			// this is used to notify the content that it needs to update its position based on new offset
			engine::event::Event<float> Scroll;

			// constructor
			ScrollBar(float contentLength, float viewportLength, bool isHorizontal);

			// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset is changed.
			void SetOffset(float offset);

			// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
			float Offset() const;

			// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
			void SetContentLength(float length);

			// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
			void SetViewportLength(float length);

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

				//void Draw(const UIDrawContext& context) const override
				//{
				//	context.skin.DrawContent(*this, context);
				//}
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

	}
}