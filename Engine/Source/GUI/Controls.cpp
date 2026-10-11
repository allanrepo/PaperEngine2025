#include <GUI/Controls.h>

#include <memory>
#include <algorithm>

namespace engine
{
	namespace gui
	{
#pragma region // namespaces

#pragma endregion

#pragma region // Label
		// this is fired up when this widget is added to a widget tree with a UI system. 
		bool Label::OnRegisterToSystem() 
		{
			// refresh cached information about text with new font type
			return RefreshLayout();
		}

		// this is fired up when this widget is removed from a widget tree with a UI system. 
		bool Label::OnUnregisterToSystem() 
		{
			// refresh cached information about text with new font type
			return RefreshLayout();
		}

		// when font resource change in UI system this widget is registered to, this callback is called
		void Label::OnResourceChange() 
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		// when this widget is resized, text position must be updated
		void Label::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		// calculates the position of the text given the font and alignment
		bool Label::RefreshLayout()
		{
			// if this widget is not part of a UI tree, bail out
			UISystem* system = GetSystem();
			if (!system)
			{
				m_textSize = {};
				m_textPosition = {};
				return false;
			}

			// if the system does not have appropriate font for the given type, bail out
			Font font = system->GetFont(m_fontType);
			if (!font.IsValid())
			{
				m_textSize = {};
				m_textPosition = {};
				return false;
			}

			// cache the text size based on the font
			m_textSize = font.GetSize(m_text);

			// reset position value to default before realigning it
			m_textPosition = {};

			switch (m_vAlign)
			{
			case VerticalAlignment::Center:
				m_textPosition.y = (GetSize().height - m_textSize.height) / 2.0f;
				break;
			case VerticalAlignment::Top:
				m_textPosition.y = 0;
				break;
			case VerticalAlignment::Bottom:
				m_textPosition.y = GetSize().height - m_textSize.height;
				break;
			default:
				break;
			}

			switch (m_hAlign)
			{
			case HorizontalAlignment::Center:
				m_textPosition.x = (GetSize().width - m_textSize.width) / 2.0f;
				break;
			case HorizontalAlignment::Left:
				m_textPosition.x = 0;
				break;
			case HorizontalAlignment::Right:
				m_textPosition.x = GetSize().width - m_textSize.width;
				break;
			default:
				break;
			}

			return true;
		}

		// constructor
		Label::Label(const std::string& text, UIResources::FontType fontType) 
			: m_text(text)
			, m_fontType(fontType)
			, m_vAlign(VerticalAlignment::Center)
			, m_hAlign(HorizontalAlignment::Center)
			, m_textSize({ 0,0 })
			, m_textPosition({ 0,0 })
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		// absolute position of text's top-left corner
		PositionF Label::GetTextAbsolutePosition() const
		{
			return GetAbsolutePosition() + m_textPosition;
		}

		// set font type
		void Label::SetFontType(UIResources::FontType type)
		{
			m_fontType = type;

			// refresh cached information about text with new font type
			RefreshLayout();
		}

		// get font type
		UIResources::FontType Label::GetFontType() const
		{
			return m_fontType;
		}

		// get text
		std::string Label::Get() const
		{
			return m_text;
		}

		// set text
		void Label::Set(const std::string& text)
		{
			m_text = text;

			RefreshLayout();
		}

		// set alignment
		void Label::SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign)
		{
			m_vAlign = vAlign;
			m_hAlign = hAlign;
			RefreshLayout();
		}

		// draw
		void Label::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawLabel(*this, context);
		}
#pragma endregion

#pragma region // Image
		// this is fired up when this widget is added to a widget tree with a UI system. 
		bool Image::OnRegisterToSystem()
		{
			RefreshLayout();
			return true;
		}

		// reposition the image based on its alignment
		void Image::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) 
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		// calculates the position of the image based on alignment
		bool Image::RefreshLayout()
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
				case VerticalAlignment::Center:
					m_imagePosition.y = (GetSize().height - m_image->GetSprite().GetHeight()) / 2.0f;
					break;
				case VerticalAlignment::Top:
					m_imagePosition.y = 0;
					break;
				case VerticalAlignment::Bottom:
					m_imagePosition.y = GetSize().height - m_image->GetSprite().GetHeight();
					break;
				default:
					break;
				}

				switch (m_hAlign)
				{
				case HorizontalAlignment::Center:
					m_imagePosition.x = (GetSize().width - m_image->GetSprite().GetWidth()) / 2.0f;
					break;
				case HorizontalAlignment::Left:
					m_imagePosition.x = 0;
					break;
				case HorizontalAlignment::Right:
					m_imagePosition.x = GetSize().width - m_image->GetSprite().GetWidth();
					break;
				default:
					break;
				}
			}

			return true;
		}

			// constructor for Renderable as image resource
		Image::Image(Renderable renderable)
				: m_image(std::make_unique<Renderable>(std::move(renderable)))
				, m_vAlign(VerticalAlignment::Center)
				, m_hAlign(HorizontalAlignment::Center)
				, m_imagePosition({ 0,0 })
			{
				m_moveBehavior = MoveBehavior::None;
				RefreshLayout();
				m_hitTestBehavior = HitTestBehavior::AlwaysFail;
			}

			// constructor for Animated as image resource
		Image::Image(Animated animated)
				: m_image(std::make_unique<Animated>(std::move(animated)))
				, m_vAlign(VerticalAlignment::Center)
				, m_hAlign(HorizontalAlignment::Center)
				, m_imagePosition({ 0,0 })
			{
				m_moveBehavior = MoveBehavior::None;
				RefreshLayout();
				m_hitTestBehavior = HitTestBehavior::AlwaysFail;
			}


			void Image::EnableStretch(bool stretch)
			{
				m_stretch = stretch;
				RefreshLayout();
			}

			bool Image::IsStretched() const
			{
				return m_stretch;
			}

			// absolute position of image's top-left corner
			PositionF Image::GetImageAbsolutePosition() const
			{
				return GetAbsolutePosition() + m_imagePosition;
			}

			// get sprite 
			Sprite Image::Get() const
			{
				return m_image->GetSprite();
			}

			// draw
			void Image::Draw(const UIDrawContext& context) const
			{
				context.skin.DrawImage(*this, context);
			}

			// set alignment
			void Image::SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign)
			{
				m_vAlign = vAlign;
				m_hAlign = hAlign;
				RefreshLayout();
			}
#pragma endregion

#pragma region // Image
		Frame::Frame(bool movable, bool droppable, bool focusable, bool interactive)
		{
			m_moveBehavior = movable ? MoveBehavior::Free : MoveBehavior::None;
			m_droppable = droppable;
			m_focusable = focusable;
			m_hitTestBehavior = interactive ? HitTestBehavior::Normal : HitTestBehavior::AlwaysFail;
		}

		void Frame::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawFrame(*this, context);
		}
#pragma endregion

#pragma region // Button
		Button::Button()
		{
			m_moveBehavior = MoveBehavior::None;
		}

		void Button::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawButton(*this, context);
		}

		void Button::OnMouseUp(const PositionF& position)
		{
			// did the mouse release occur over this button? if not, then this mouse up is not for us. ignore
			if (!Contains(position)) return;

			// handle click event
			Click();
		}
#pragma endregion

#pragma region // Thumb
		Thumb::Thumb()
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail; // non interactive
		}

		Thumb::Thumb(bool isHorizontal)
		{
			m_moveBehavior = isHorizontal ? MoveBehavior::Horizontal : MoveBehavior::Vertical;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail; // non interactive
		}

		void Thumb::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawThumb(*this, context);
		}
#pragma endregion

#pragma region // Slider
		void Slider::OnMouseDown(const PositionF& position)
		{
			m_isDragging = true;
			UpdateValueFromPosition(position);
		}

		void Slider::OnMouseUp(const PositionF& position) 
		{
			m_isDragging = false;
		}

		void Slider::OnMouseMove(const PositionF& position)
		{
			if (m_isDragging)
			{
				UpdateValueFromPosition(position);
			}
		}

		void Slider::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) 
		{
			UpdateThumbSize();

			UpdateThumbPosition();
		}

		void Slider::UpdateThumbPosition()
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

		void Slider::UpdateThumbSize()
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

		void Slider::UpdateValueFromPosition(const PositionF& position)
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

		float Slider::Snap(float value) const
		{
			if (m_steps <= 1) return m_min;

			int steps = m_steps - 1;

			float stepSize = (m_max - m_min) / steps;

			value = std::round((value - m_min) / stepSize);
			value *= stepSize;
			value += m_min;

			return value;
		}

		Slider::Slider(float min, float max, float thumbLength) :
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

		void Slider::Horizontal(bool enable)
		{
			if (m_horizontal != enable)
			{
				m_horizontal = enable;

				UpdateThumbSize();

				UpdateThumbPosition();
			}
		}

		bool Slider::Horizontal() const
		{
			return m_horizontal;
		}

		void Slider::SetThumbLength(float length)
		{
			m_thumbLength = length;

			UpdateThumbSize();

			UpdateThumbPosition();
		}

		void Slider::Min(float min)
		{
			m_min = min;

			// range changed, we might need to update value if it gets clamped
			Value(m_value);

			// value might not have changed, but thumb position might change with new range
			UpdateThumbPosition();
		}

		void Slider::Max(float max)
		{
			m_max = max;

			// range changed, we might need to update value if it gets clamped
			Value(m_value);

			// value might not have changed, but thumb position might change with new range
			UpdateThumbPosition();
		}

		float Slider::Min() const
		{
			return m_min;
		}

		float Slider::Max() const
		{
			return m_max;
		}

		void Slider::Value(float value)
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
			Change(m_value);
		}

		float Slider::Value() const
		{
			return m_value;
		}

		void Slider::SetStepSize(float size)
		{
			m_steps = size > 0 ?
				static_cast<int>(std::round((m_max - m_min) / size)) + 1 :
				1;

			// snap the current value and update value. this will also possibly update thumb position if needed
			Value(Snap(m_value));
		}

		void Slider::SetStepCount(int count)
		{
			m_steps = count;

			// snap the current value and update value. this will also possibly update thumb position if needed
			Value(Snap(m_value));
		}

		void Slider::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawSlider(*this, context);
		}
#pragma endregion

#pragma region // ScrollBar
		// checks if the scrollbar can scroll. if content is smaller than viewport, it cannot scroll
		bool ScrollBar::CanScroll() const
		{
			return m_contentLength > m_viewportLength;
		}

		// callback when mouse is pressed down on the scroll bar. this can happen on the thumb or on the track. 
		// if it is on the track, we will move the thumb to that position and start dragging. 
		// if it is on the thumb, we will start dragging the thumb.
		void ScrollBar::OnMouseDown(const PositionF& pos)
		{
			// if cannot scroll e.g. content smaller than viewport, no need to drag
			if (!CanScroll()) return;

			// did we clicked on thumb or track?
			float lpos = (m_horizontal ? pos.x : pos.y) - (m_horizontal ? GetAbsolutePosition().x : GetAbsolutePosition().y);
			float thumbSize = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;
			float thumbPos = m_horizontal ? m_thumb->GetPosition().x : m_thumb->GetPosition().y;

			// if click on track, move thumb to that position, and start dragging. the thumb will be centered on the click position if possible.
			if (lpos < thumbPos || lpos > thumbPos + thumbSize)
			{
				float newThumbPos = lpos - thumbSize / 2.0f;
				m_thumb->SetPosition(m_horizontal ? PositionF{ newThumbPos,  m_thumb->GetPosition().y } : PositionF{ m_thumb->GetPosition().x, newThumbPos });
			}

			// set dragging state to true and forward the mouse down event to the thumb so it can handle dragging
			m_isDragging = true;
			m_thumb->MouseDown(pos);
		}

		// callback when mouse is released on the scroll bar. this can happen on the thumb or on the track.
		void ScrollBar::OnMouseUp(const PositionF&) 
		{
			m_isDragging = false;
		}

		// callback when mouse is moved on the scroll bar. this can happen on the thumb or on the track.
		void ScrollBar::OnMouseMove(const PositionF& pos)
		{
			// if we're dragging, let the thumb handle the mouse move event so it can update its position and fire its moved event
			if (m_isDragging)
			{
				m_thumb->MouseMove(pos);
			}
		}

		// internal method to update the size of the thumb based on the content length, viewport length, and scrollbar length. this is called when content or viewport changes.
		void ScrollBar::UpdateThumbSize()
		{
			// get the length of scrollbar based on its orientation.
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

		// internal method to update the position of the thumb when offset, content length, or viewport length changes.
		void ScrollBar::UpdateThumbPosition()
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

		// callback when the size of the scrollbar changes. this is called when the scrollbar is resized. we will update the thumb size and position based on new size
		void ScrollBar::OnSizeChanged(const SizeF&, const SizeF&)
		{
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		// constructor
		ScrollBar::ScrollBar(float contentLength, float viewportLength, bool isHorizontal) :
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
			m_thumb->Moved += [this](const PositionF& newPos)
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

		// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset is changed.
		void ScrollBar::SetOffset(float offset)
		{
			// clamp offset to be within 0 and (contentLength - viewportLength). if content is smaller than viewport, set offset to 0
			offset = std::clamp(offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// if thumb position did not change, no need to update and invoke scroll events
			if (m_offset == offset) return;

			// update offset, thumb position, and fire scroll event
			m_offset = offset;
			UpdateThumbPosition();
			Scroll(m_offset);
		}

		// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
		float ScrollBar::Offset() const
		{
			return m_offset;
		}

		// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
		void ScrollBar::SetContentLength(float length)
		{
			// cannot be negative content length. if negative, set to 0
			m_contentLength = std::max<float>(0.0f, length);

			// refresh offset in case it is out of range now due to content length change. if content is smaller than viewport, set offset to 0
			m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// update thumb size and position based on possibly new content length and offset
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
		void ScrollBar::SetViewportLength(float length)
		{
			// cannot be negative viewport length. if negative, set to 0
			m_viewportLength = std::max<float>(0.0f, length);

			// refresh offset in case it is out of range now due to content length change. if content is smaller than viewport, set offset to 0
			m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// update thumb size and position based on possibly new content length and offset
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		float ScrollBar::GetContentLength() const
		{
			return m_contentLength;
		}
		float ScrollBar::GetViewportLength() const
		{
			return m_viewportLength;
		}

		void ScrollBar::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawScrollBar(*this, context);
		}
#pragma endregion

#pragma region // ResizeableFrame
		ResizeableFrame::Grip::Grip(bool MoveHorizontal, bool MoveVertical)
		{
			m_moveBehavior = (MoveHorizontal && MoveVertical) ? MoveBehavior::Free :
				(MoveHorizontal && !MoveVertical) ? MoveBehavior::Horizontal :
				(!MoveHorizontal && MoveVertical) ? MoveBehavior::Vertical :
				MoveBehavior::None;

			m_droppable = false;
			m_focusable = false;
		}

		void ResizeableFrame::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) 
		{
			UpdateLayout();
		}

		void ResizeableFrame::UpdateLayout()
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

		void ResizeableFrame::UpdateContentLayout()
		{
			m_content->SetPosition({ m_borderSize, m_borderSize });
			m_content->SetSize({ GetSize().width - m_borderSize * 2, GetSize().height - m_borderSize * 2 });
		}

		void ResizeableFrame::UpdateTopRightGripLayout()
		{
			// resize and reposition right grip control to occupy top-right corner of the frame with border size as thickness
			m_topRightResizeGrip->SetPosition({ m_size.width - m_borderSize, 0 });
			m_topRightResizeGrip->SetSize({ m_borderSize, m_borderSize });
		}

		void ResizeableFrame::UpdateTopGripLayout()
		{
			// resize and reposition right grip control to occupy top edge of the frame with border size as thickness
			m_topResizeGrip->SetPosition({ m_borderSize, 0 });
			m_topResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });
		}

		void ResizeableFrame::UpdateRightGripLayout()
		{
			// resize and reposition right grip control to occupy right edge of the frame with border size as thickness
			m_rightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_borderSize });
			m_rightResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });
		}

		void ResizeableFrame::UpdateTopLeftGripLayout()
		{
			// resize and reposition right grip control to occupy top-left corner of the frame with border size as thickness
			m_topLeftResizeGrip->SetPosition({ 0, 0 });
			m_topLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });
		}

		void ResizeableFrame::UpdateLeftGripLayout()
		{
			// resize and reposition right grip control to occupy left edge of the frame with border size as thickness
			m_leftResizeGrip->SetPosition({ 0, m_borderSize });
			m_leftResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });
		}

		void ResizeableFrame::UpdateBottomRightGripLayout()
		{
			// resize and reposition right grip control to occupy bottom-right corner of the frame with border size as thickness
			m_bottomRightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_size.height - m_borderSize });
			m_bottomRightResizeGrip->SetSize({ m_borderSize, m_borderSize });
		}

		void ResizeableFrame::UpdateBottomGripLayout()
		{
			// resize and reposition right grip control to occupy bottom edge of the frame with border size as thickness
			m_bottomResizeGrip->SetPosition({ m_borderSize, m_size.height - m_borderSize });
			m_bottomResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });
		}

		void ResizeableFrame::UpdateBottomLeftGripLayout()
		{
			// resize and reposition right grip control to occupy bottom-left corner of the frame with border size as thickness
			m_bottomLeftResizeGrip->SetPosition({ 0, m_size.height - m_borderSize });
			m_bottomLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });
		}

		SizeF ResizeableFrame::ClampSize(const SizeF& size) const
		{
			return
			{
				std::max<float>(size.width,  m_minResize.width),
				std::max<float>(size.height, m_minResize.height)
			};
		}

		ResizeableFrame::ResizeableFrame(float borderSize, const SizeF& minSize) :
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
				m_bottomRightResizeGrip->DragBegin += capture;
				m_topLeftResizeGrip->DragBegin += capture;
				m_bottomResizeGrip->DragBegin += capture;
				m_bottomLeftResizeGrip->DragBegin += capture;
				m_topRightResizeGrip->DragBegin += capture;
				m_topResizeGrip->DragBegin += capture;
				m_leftResizeGrip->DragBegin += capture;
				m_rightResizeGrip->DragBegin += capture;
			}

			// everytime grip moves, we make sure it always stay at the position relative to frame all the time.
			// sometimes when you drag a grip, and the drag change is same as before, resizeable frame size does not change. 
			// this will not trigger resizeableframe's OnSizeChange and therefore will not exacute LayoutUpdate. the grip will then be out of position.
			// repositioning it here ensures that everytime grip moves, it will be repositioned back to its supposed location relative to resizeableframe
			// the impact though is that it can reposition the grip multiple times per move. if it so happens resizeableframe is resized, it will
			// execute LayoutUpdate which will reposition the grip. then this handle will again reposition it. it won't cause recursive chain since
			// SetPosition() is guarded. but it costs CPU execution time as grip's SetPosition() can be called more than once.
			{
				m_topRightResizeGrip->Moved += [&](const PositionF& pos) { UpdateTopRightGripLayout(); };
				m_topResizeGrip->Moved += [&](const PositionF& pos) { UpdateTopGripLayout(); };
				m_topLeftResizeGrip->Moved += [&](const PositionF& pos) { UpdateTopLeftGripLayout(); };
				m_rightResizeGrip->Moved += [&](const PositionF& pos) { UpdateRightGripLayout(); };
				m_leftResizeGrip->Moved += [&](const PositionF& pos) { UpdateLeftGripLayout(); };
				m_bottomRightResizeGrip->Moved += [&](const PositionF& pos) { UpdateBottomRightGripLayout(); };
				m_bottomResizeGrip->Moved += [&](const PositionF& pos) { UpdateBottomGripLayout(); };
				m_bottomLeftResizeGrip->Moved += [&](const PositionF& pos) { UpdateBottomLeftGripLayout(); };
			}

			// DEBUG: keep for now. we will refer to this code later when we implement an anchor like widget 
			//// we also track content drag in case content is draggable, we bubble up movement to the frame and make content stationary
			//{
			//	m_content->DragBegin += [&](const Widget::DragEventArgs& args)
			//		{
			//			MouseDown(args.currentPosition);
			//		};

			//	m_content->DragMove += [&](const Widget::DragEventArgs& args)
			//		{
			//			MouseMove(args.currentPosition);
			//			UpdateLayout();
			//		};

			//	m_content->DragEnd += [&](const Widget::DragEventArgs& args)
			//		{
			//			MouseUp(args.currentPosition);
			//		};
			//}

			// we track grips' drag. we update resizeableframe's position and size depending on grip's drag movement
			{
				// bottom-right grip handlers
				m_bottomRightResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
					{
						VecF delta = args.Delta();

						SetSize(ClampSize(
							{
								m_beginSize.width + delta.x,
								m_beginSize.height + delta.y
							}));
					};

				//  top-left grip handlers
				m_topLeftResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
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
				m_bottomLeftResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
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
				m_topRightResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
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
				m_topResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
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
				m_leftResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
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
				m_rightResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
					{
						SetSize(ClampSize(
							{
								m_beginSize.width + args.Delta().x,
								m_beginSize.height,
							}));
					};

				//  bottom grip handlers
				m_bottomResizeGrip->DragMove += [&](const Widget::DragEventArgs& args)
					{
						SetSize(ClampSize(
							{
								m_beginSize.width,
								m_beginSize.height + args.Delta().y,
							}));
					};
			}
		}

		void ResizeableFrame::SetMinResize(const SizeF& size)
		{
			m_minResize = size;
		}

		void ResizeableFrame::SetBorderSize(float size)
		{
			m_borderSize = size;
		}

		void ResizeableFrame::Draw(const UIDrawContext& context) const 
		{
			context.skin.DrawResizeableFrame(*this, context);
		}

		void ResizeableFrame::AddContent(std::unique_ptr<Widget> widget)
		{
			m_content->AddChild(std::move(widget));
		}

		SizeF ResizeableFrame::GetContentsize() const
		{
			return m_content->GetSize();
		}

		SizeF ResizeableFrame::GetContentAreaSize() const
		{
			return SizeF{ GetSize().width - m_borderSize * 2, GetSize().height - m_borderSize * 2 };
		}
#pragma endregion

#pragma region // Panel - non drawable, non focusable, none droppable, non movable, but interactive widget designed to hold child widgets and manage its layout in group.
		Panel::Panel(bool interactive)
		{
			m_focusable = false;
			m_droppable = false;
			m_moveBehavior = Widget::MoveBehavior::None;
			m_hitTestBehavior = interactive ? Widget::HitTestBehavior::Normal : Widget::HitTestBehavior::AlwaysFail;
		}
#pragma endregion

#pragma region // DragHandle - handle its drag state and delegate the drag movement to its referenced proxy widget
		DragHandle::DragHandle(Widget& widget, bool moveHorizontal, bool moveVertical)
			: m_widget(widget)
			, m_isMoving(false)
			, m_beginMovePosition(0, 0)
		{
			// set move behavior
			m_moveBehavior =
				moveHorizontal ?
				moveVertical ?
				Widget::MoveBehavior::Free :		// if both horizontal and vertical movement is true...
				Widget::MoveBehavior::Horizontal :	// if only horizontal movement is true...
				moveVertical ?
				Widget::MoveBehavior::Vertical :	// if only vertical movement is true...
				Widget::MoveBehavior::None;			// if neither movement is true...


			// we listen to this widget's drag event when drag starts. we capture this widget's position as the starting position of the drag
			DragBegin += [&](const Widget::DragEventArgs& args)
				{
					// remember the widget's position now. this will be the reference position as it gets dragged around by mouse
					m_beginMovePosition = m_widget.GetPosition();

					// this widget is now moving
					m_isMoving = true;
				};

			// we listen to this widget's drag event when drag is in progress. here we calculate the change in position between current mouse position
			// and the position when drag started. we then apply this delta to the proxy widget's position to move it around
			DragMove += [&](const Widget::DragEventArgs& args)
				{
					if (m_isMoving)
					{
						// we only drag the widget if new position after this drag is different from current position
						PositionF dragpos = m_beginMovePosition + args.Delta();
						if (dragpos == m_widget.GetPosition()) return;

						// transform widget's position based on mouse movement delta
						m_widget.SetPosition(dragpos);
					}
				};

			// this ends the drag operation.
			DragEnd += [&](const Widget::DragEventArgs& args)
				{
					// if we never moved, we shouldn't be ending move. we shouldn't be firing up DragEnd event
					if (!m_isMoving) return;

					m_isMoving = false;
				};
		}
#pragma endregion

	}

}