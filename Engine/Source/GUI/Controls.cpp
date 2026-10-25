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
			return UpdateLayout();
		}

		// this is fired up when this widget is removed from a widget tree with a UI system. 
		bool Label::OnUnregisterToSystem() 
		{
			// refresh cached information about text with new font type
			return UpdateLayout();
		}

		// when font resource change in UI system this widget is registered to, this callback is called
		void Label::OnResourceChange() 
		{
			// refresh cached information about text with new font type
			UpdateLayout();
		}

		// when this widget is resized, text position must be updated
		void Label::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			// refresh cached information about text with new font type
			UpdateLayout();
		}

		// calculates the position of the text given the font and alignment
		bool Label::UpdateLayout()
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
			UpdateLayout();
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

			UpdateLayout();
		}

		// set alignment
		void Label::SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign)
		{
			m_vAlign = vAlign;
			m_hAlign = hAlign;
			UpdateLayout();
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
			UpdateLayout();
			return true;
		}

		// reposition the image based on its alignment
		void Image::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) 
		{
			// refresh cached information about text with new font type
			UpdateLayout();
		}

		// calculates the position of the image based on alignment
		bool Image::UpdateLayout()
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
				UpdateLayout();
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
				UpdateLayout();
				m_hitTestBehavior = HitTestBehavior::AlwaysFail;
			}


			void Image::EnableStretch(bool stretch)
			{
				m_stretch = stretch;
				UpdateLayout();
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
				UpdateLayout();
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
			return (m_contentLength > m_viewportLength) && (m_viewportLength > 0) && (m_contentLength > 0);
		}

		// callback when mouse is pressed down on the scroll bar. this can happen on the thumb or on the track. 
		// if it is on the track, we will move the thumb to that position and start dragging. 
		// if it is on the thumb, we will start dragging the thumb.
		void ScrollBar::OnMouseDown(const PositionF& pos)
		{
			// if cannot scroll e.g. content smaller than viewport, no need to drag
			if (!CanScroll()) return;

			// get the local position in the scrollbar's area where the mouse clicked. we only need position along the scrollbar's orientation.
			float lpos = (m_horizontal ? pos.x : pos.y) - (m_horizontal ? GetAbsolutePosition().x : GetAbsolutePosition().y);

			// get the size of the thumb. this is the length of the thumb widget along scrollbar's orientation
			float thumbSize = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;

			// get the thumb position (local to scrollbar). this is the top position of thumb widget if scrollbar orientation is vertical, and left position if horizontal
			float thumbPos = m_horizontal ? m_thumb->GetPosition().x : m_thumb->GetPosition().y;

			// did we clicked on thumb or track?
			if (lpos < thumbPos || lpos > thumbPos + thumbSize)
			{
				// if click on track, move thumb to that position, and start dragging. the thumb will be centered on the click position if possible.
				float newThumbPos = lpos - thumbSize / 2.0f;

				// the new thumb pos could position thumb outside scrollbar's tracklength as the 'pos' from OnMouseDown could be outside scrollbar's area
				// that's fine. we just set it. we will validate the thumb position with our handler to thumb's 'Moved' event
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
		// thumb moves when this happen
		//	- thumb moves just a normal widget would.no checking if it's position is outside scrollbar's scroll range
		//	- scrollbar handles thumb move event.without checking if position is valid or not, it forces it to be inside scrollbar's scroll range
		//	- translate current thumb physical pos into logical pos
		//	- if logical pos is outside logical range, clamp it
		//	- translate logical pos back to physical thumb pos
		//	- now compare this translated physical thumb pos with thumb's current physical pos
		//		- if it's different, it means the thumb need to be nudged so it's really inside scrollbar's scroll range
		//		- set thumb physical position to the newly translated physical pos
		//		- setting thumb physical position will trigger thumb move event.
		//		- this must be ignored
		//		- the handler's purpose is to correct the thumb's position to ensure it's inside scrollbar's scroll range
		//		- we already guaranteed that this position puts thumb inside scrollbar's scroll range. so no need to correct it again
		//		- we also should skip raising Scroll event because we're doing that already
		//	- if logical pos != offset
		//		- offset = logical pos
		//		- raise Scroll event
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

			// get the minimum and maximum allowed size of the thumb. it cannot be smaller than minimum thumblength, unless tracklength is smaller than minimum thumb length
			float min = std::min<float>(m_minThumbLength, trackLength);
			float max = trackLength;

			// here when calculating the actual length of the thumb, we clamp to tracklength so even if ratio > 1, we don't end up with thumb bigger than scroll bar
			float length = std::clamp(ratio * trackLength,
				// we're comparing value between m_minThumbLength and trackLength. between m_minThumbLength and trackLength, use smaller for min value and bigger for max value. 
				// this is a must because if min value is bigger than max value, std::clamp will throw an exception
				min, max
			); // clamp min size

			SizeF thumbSize{
				m_horizontal ? length : thickness,
				m_horizontal ? thickness : length
			};
			m_thumb->SetSize(thumbSize);
		}

		// internal method to update the physical position of the thumb based on current offset, content length, or viewport length.
		void ScrollBar::UpdateThumbPosition()
		{
			// trackLength is the physical length the thumb can move within the scrollbar. this must be length of scrollbar minus length of thumb. 
			float trackLength = (m_horizontal ? GetSize().width : GetSize().height) - (m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height);

			// this is the logical range in which viewport can move. if value is 1, it means viewport can be at logical position 0 and 1.
			float logicalRange = m_contentLength - m_viewportLength;

			// if viewport length is greater than content length, logical range is < 0. scrolling is NOT required
			logicalRange = logicalRange < 0 ? 0 : logicalRange;

			// now let's calculate the thumb's physical position by translating its logical position
			// note that if logical range is <= 0, thumb is not scrolling. therefore it will be set to 0
			float thumbPhysicalPos = logicalRange == 0 ? 0.0f : m_offset * trackLength / logicalRange;
			m_thumb->SetPosition(m_horizontal ? PositionF{ thumbPhysicalPos, 0 } : PositionF{ 0, thumbPhysicalPos });
		}

		// callback when the size of the scrollbar changes. this is called when the scrollbar is resized. we will update the thumb size and position based on new size
		//	note: offset, viewport length, and content length does not change when scrollbar resize
		//	- scrollbar length represents content length while thumb length represents viewport length
		//		- if scrollbar length is 100, content length is 4, and viewport length is 3, then thumb length is 75
		//		- if scrollbar length changes, then thumb length must change to maintain its proportion with content and viewport length
		//	- likewise, thumb's physical position represents offset value (logical position)
		//		- if scrollbar length and thumb length changes, thumb's physical position needs to be updated too in order to maintain its proportion with offset value 
		//			- repositioning thumb will trigger thumb move.
		//				- it's possible that thumb moves in physical position where its corresponding logical position is not same with current offset value
		//				- e.g.scrollbar length is too small and thumb length is already at minimum length
		//					- thumb length will be resized to fill the scrollbar.its physical position will be set to 0, 0
		//					- physical position of 0, 0 means logical position of 0. offset must not be changed to 0 if it's value is not 0.
		//				- since we're not supposed to change offset value, thumb's move handler must not change offset and must not raise Scroll event
		void ScrollBar::OnSizeChanged(const SizeF&, const SizeF&)
		{
			// update thumb size
			UpdateThumbSize();

			// offset must not change value since only scrollbar size changed. 
			// yes the thumb position must be updated since the tracklength has changed but this should not change offset value.
			// if tracklength becomes smaller than minimum thumb size, the thumb resizes to fit the whole tracklength and its position to 0,0
			// this change in thumb position must not change our offset. 
			m_ignoreScrollBarScroll = true;

			// update the thumb position
			UpdateThumbPosition();

			// after updating thumb position, we don't need to suppress updating offset
			m_ignoreScrollBarScroll = false;
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
					// we might be setting position of thumb but we know it's position is correct and does not need recorrection, so we just ignore this handler.
					if (m_ignoreThumbMove) return;

					// trackLength is the physical length the thumb can move within the scrollbar. this must be length of scrollbar minus length of thumb. 
					float trackLength = (m_horizontal ? GetSize().width : GetSize().height) - (m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height);

					// this is the logical range in which viewport can move. if value is 1, it means viewport can be at logical position 0 and 1.
					float logicalRange = m_contentLength - m_viewportLength;

					// if viewport length is greater than content length, logical range is < 0. scrolling is NOT required
					logicalRange = logicalRange < 0 ? 0 : logicalRange;

					// get the thumb position
					float thumbPhysicalPos = m_horizontal ? newPos.x : newPos.y;

					// thumb position is in physical space and could be any value. let's translate it into a logical space
					// note the +0.5 is just for centering
					// if tracklength is 0. this means thumb has no space to move. the thumb fills up the scrollbar. 
					// since thumb cannot scroll, it will be positioned at 0,0
					float thumbLogicalPos = trackLength == 0 ? 0 : std::floor(thumbPhysicalPos * logicalRange / trackLength + 0.5f);

					// thumb's logical position is now in logical space, but it can be outside of the logical range. let's clamp it to within logical range
					thumbLogicalPos = std::clamp<float>(thumbLogicalPos, 0.0f, std::max<float>(0.0f, logicalRange));

					// we now have a valid logical position for our thumb. translate it back to physical space
					// note that if logical range is <= 0, thumb is not scrolling. therefore it will be set to 0
					float correctedThumbPhysicalPos = logicalRange == 0 ? 0.0f : thumbLogicalPos * trackLength / logicalRange;

					// if corrected position is not same as current position, it means we need to adjust the thumb position to be within scrollbar's scroll range
					if (correctedThumbPhysicalPos != thumbPhysicalPos)
					{
						// finally, we set the thumb position. 
						// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
						m_ignoreThumbMove = true;
						m_thumb->SetPosition(m_horizontal ? PositionF{ correctedThumbPhysicalPos, 0 } : PositionF{ 0, correctedThumbPhysicalPos });
						m_ignoreThumbMove = false;
					}

					// if we only intend to reposition thumb but not change offset, bail out now
					if (m_ignoreScrollBarScroll) return;

					// finally, if the calculated index is same as current offset, we don't have to do anything
					// note that offset is just another name for index.
					if (m_offset == thumbLogicalPos) return;
					m_offset = thumbLogicalPos;
					Scroll(m_offset);
				};
		}

		// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset is changed.
		// since we're changing offset, we can immediately check offset if valid and clamp it if it's not.then if resulting offset is same as current, can bail out now
		//	- at this point, our new offset is guaranteed valid, meaning if translated to thumb physical position, thumb will be inside scrollbar's scroll range
		//	- also if we reach this point, new offset is different from current offset so we need to change its value
		//	- changing offset value means we need to update thumb physical position
		//		- but resulting physical position guarantees thumb is inside scrollbar's scroll range so we don't need to check for recorrecting.
		//		- this will ignore thumb's move event
		//	- since we're ignoring thumb's move event, we need to raise Scroll event ourselves.
		//	- Scroll event needs to be raised because offset value changed
		void ScrollBar::SetOffset(float offset)
		{
			// clamp offset to be within logical range (0 and [contentLength - viewportLength]). if content is smaller than viewport, set offset to 0
			offset = std::clamp(offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// if the new offset is same as current, can just bail out
			if (m_offset == offset) return;

			// update offset, thumb position, and fire scroll event
			m_offset = offset;

			// at this point, we know new offset is guaranteed to be valid, so we can set thumb position without worry that it will be in an invalid position
			m_ignoreThumbMove = true;

			// now update the thumb's physical position based on new offset.
			// since we are ignoring recorrection, it will just blindly set thumb to new physical position without validating
			UpdateThumbPosition();

			// we can now set state back to normal and handle any thumb movement. 
			m_ignoreThumbMove = false;

			// since we ignore thumb movement, we will raise scroll ourselves. we know offset has changed at this point, so we just raise the event
			Scroll(m_offset);
		}

		// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
		float ScrollBar::Offset() const
		{
			return m_offset;
		}

		// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
		// change content length
		//	- because content changes, logical range changes too.This needs to check if offset needs clamping.
		//	- since content length changes, proportion with thumb and scrollbar length changes.
		//		- we need to update thumb size since scrollbar length did not change, so we can maintain proportion
		//	- when content length changes, ratio between content and viewport changes.
		//		- it's possible that offset changes too. current offset that is within logical range might be outside of it already and needs clamping
		//			- e.g.current offset = 5, vp = 1, content = 8. now content changes to 4. offset needs to be clamped to 4
		//			- this needs thumb position to be updated
		//		- offset may not need to change but still needs thumb position to change.
		//			- e.g.offset = 2, vp = 1, content = 8. scrollbar length = 800, so thumb length = 100 and thumb position = 200
		//			- but content changes to 5. vp is still 1 and offset remains 2 (still within logical range after content changes to 5).
		//			- scrollbar length is still 800 but thumb length now changes to 160. even though offset remains at 2, thumb position now needs to change to 320
		void ScrollBar::SetContentLength(float length)
		{
			// clamp the new length to be sure its valid. it cannot be negative.
			length = std::max<float>(0.0f, length);

			// bail out if new length is same as current content length
			if (length == m_contentLength) return;

			// let's now change to new length
			m_contentLength = length;

			// since content length changes, proportion with thumb and scrollbar length changes. update thumb size to maintain the proportion
			UpdateThumbSize();

			// store the old offset. we need this as reference later to check if we need to raise Scroll event because offset changed
			float oldOffset = m_offset;

			// update offset now by clamping with new content length. the result will be a valid offset (within logical range) regardless
			m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// update thumb position. offset may or may not changed but we still need to update thumb's physical position.
			// but we also can skip recorrecting because we're updating thumb position based on new offset which is guaranteed to be valid.
			m_ignoreThumbMove = true;
			UpdateThumbPosition();

			// we can now resume handling thumb move event and recorrect if needed, raise Scroll event if needed
			m_ignoreThumbMove = false;

			// since we ignored thumb move event, we did not raise Scroll event. that is fine. we want to raise Scroll event ourselves.
			// we know if offset changes or not.
			if (oldOffset != m_offset)
			{
				Scroll(m_offset);
			}
		}

		// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
		void ScrollBar::SetViewportLength(float length)
		{
			// clamp the new length to be sure its valid. it cannot be negative.
			length = std::max<float>(0.0f, length);

			// bail out if new length is same as current content length
			if (length == m_viewportLength) return;

			// let's now change to new length
			m_viewportLength = length;

			// since viewport length changes, proportion with thumb and scrollbar length changes. update thumb size to maintain the proportion
			UpdateThumbSize();

			// store the old offset. we need this as reference later to check if we need to raise Scroll event because offset changed
			float oldOffset = m_offset;

			// update offset now by clamping with new viewport length. the result will be a valid offset (within logical range) regardless
			m_offset = std::clamp(m_offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));

			// update thumb position. offset may or may not changed but we still need to update thumb's physical position.
			// but we also can skip recorrecting because we're updating thumb position based on new offset which is guaranteed to be valid.
			m_ignoreThumbMove = true;
			UpdateThumbPosition();

			// we can now resume handling thumb move event and recorrect if needed, raise Scroll event if needed
			m_ignoreThumbMove = false;

			// since we ignored thumb move event, we did not raise Scroll event. that is fine. we want to raise Scroll event ourselves.
			// we know if offset changes or not.
			if (oldOffset != m_offset)
			{
				Scroll(m_offset);
			}
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

#pragma region // ViewPort - container widget that holds a single "content" widget and manages the content widget's scrolling.
		// design notes:
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
		//		- it is the distance between viewport's origin (0,0) and content's top-left position.
		//		- since content's top-left position cannot be greater than 0,0 this value is 0,0 or -x, -y
		//		- what happens when you set offset?
		//			- 
		// 
		// 
		// 
		
		// this method ensures that content's position is always within the viewport's bounds. 
		// if content's position is outside the viewport, it will be moved back to the nearest position within the viewport
		bool ScrollView::ViewPort::UpdateContentPosition()
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

			// only set position of content if confirmed its position really needs to be changed
			if (updatePos)
			{
				m_content->SetPosition(position);
				return true;
			}

			// if you reach this point, content position need not be changed because it did not need to be corrected
			return false;
		}

		// handler for when viewport's size changes. we need to ensure that content's position is still within the viewport's       
		void ScrollView::ViewPort::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			UpdateContentPosition();
		}

		// event handler for when content's size changes. we need to ensure that content's position is still within the viewport's bounds
		void ScrollView::ViewPort::OnContentSizeChanged(const SizeF& size)
		{
			// we need to ensure that the content is within the viewport's bounds after resizing
			UpdateContentPosition();

			// bubble up the event to notify that content's size has changed. this is useful for scroll bars to update their thumb size and position
			ContentSizeChanged(size);
		}

		// callback when content change position. 
		void ScrollView::ViewPort::OnContentMove(const PositionF& pos)
		{
			// When content moves, ensure it remains within the viewport bounds.
			// 
			// 'Moved' event from content is only raised when content's position really changed.
			// Update the viewport's layout to ensure content stays within the bounds of viewport's area.
			// Updating the layout may result in setting content's position again which will result in raising content's 'Moved' event.
			// As Moved will only be raised  when content's position really changed, this will not result in recursive loop
			//
			// also note that since we are monitoring content's move event, we don't need to monitor its drag event as dragging will 
			// also eventually set position of content and will raise Moved
			// 
			// if this function returns true, it means it corrected the position. 
			if (UpdateContentPosition())
			{
				return;
			}

			// fire scroll event to notify that content has moved and viewport's offset has changed
			Scroll(GetOffset());
		}

		ScrollView::ViewPort::ViewPort()
			:m_content(nullptr)
		{
			// this widget should not be movable. it also should not be focusable because it is a container for a content that is the actual widget this represents
			m_moveBehavior = MoveBehavior::None;
			m_droppable = false;
			m_focusable = false;

			// create a default content widget and set it as the viewport's content
			std::unique_ptr<Widget> content = std::make_unique<Widget>();
			m_content = content.get();
			SetContent(std::move(content));
		}

		SizeF ScrollView::ViewPort::GetContentSize() const
		{
			return m_content->GetSize();
		}

		VecF ScrollView::ViewPort::GetOffset() const
		{
			return PositionF{ 0,0 } - m_content->GetPosition();
		}

		void ScrollView::ViewPort::SetContent(std::unique_ptr<Widget> content)
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

		void ScrollView::ViewPort::SetContentSize(const SizeF& size)
		{
			m_content->SetSize(size);
		}

		void ScrollView::ViewPort::SetOffset(const VecF& offset)
		{
			// Setting the viewport offset is done by moving the content in the
			// opposite direction relative to the viewport.
			//
			// This ultimately calls m_content->SetPosition().
			//
			// If the position actually changes, Widget::SetPosition() will fire
			// the content's Moved event. ViewPort listens to that event and
			// performs UpdateContentPosition() to enforce viewport bounds and any other
			// scrolling rules.
			// 
			// if UpdateContentPosition() recorrects the position, it will call content's m_content->SetPosition() again.
			// since position is corrected, the corresponding UpdateContentPosition() from "Moved" event handler will not 
			// call m_content->SetPosition() anymore
			//
			// Widget::SetPosition() is guarded against assigning the same value,
			// preventing redundant notifications and avoiding recursive update
			// loops when UpdateLayout() performs corrective repositioning.
			PositionF pos = PositionF{ 0,0 } - offset;
			m_content->SetPosition(pos);
		}

		void ScrollView::ViewPort::AddContent(std::unique_ptr<Widget> content)
		{
			m_content->AddChild(std::move(content));
		}
#pragma endregion

#pragma region // ScrollView
		// this will update thumb positions of the scroll bars
		void ScrollView::UpdateThumbPositions()
		{
			VecF offset = m_viewport->GetOffset();
			m_hScrollBar->SetOffset(offset.x);
			m_vScrollBar->SetOffset(offset.y);
		}

		// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
		void ScrollView::UpdateScrollBarContentSize()
		{
			SizeF contentSize = m_viewport->GetContentSize();

			m_hScrollBar->SetContentLength(contentSize.width);
			m_vScrollBar->SetContentLength(contentSize.height);
		}

		// this will update the viewport size of scroll bars.  scroll bars then will refresh its thumb size and thumb position internally
		void ScrollView::UpdateScrollBarViewportSize()
		{
			SizeF viewportSize = m_viewport->GetSize();

			m_hScrollBar->SetViewportLength(viewportSize.width);
			m_vScrollBar->SetViewportLength(viewportSize.height);
		}

		void ScrollView::UpdateLayout()
		{
			throw std::runtime_error("ScrollView error ScrollView::UpdateLayout()");

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

				bool updateViewportSize = false;

				// now let's hide or show vertical scrollbar. do this only if the visibility of the scrollbar has changed. 
				if (vScrollBarVisible)// && !m_vScrollBar->IsVisible())
				{
					m_vScrollBar->Show();
					updateViewportSize = true;
				}
				else if (!vScrollBarVisible)// && m_vScrollBar->IsVisible())
				{
					m_vScrollBar->Hide();
					updateViewportSize = true;
				}

				// now let's hide or show horizontal scrollbar. do this only if the visibility of the scrollbar has changed. 
				if (hScrollBarVisible)// && !m_hScrollBar->IsVisible())
				{
					m_hScrollBar->Show();
					updateViewportSize = true;
				}
				else if (!hScrollBarVisible)// && m_hScrollBar->IsVisible())
				{
					m_hScrollBar->Hide();
					updateViewportSize = true;
				}

				// we only set viewport's size if any scrollbar toggled its visibility. if both scrollbars are already visible or already hidden, we don't need to set viewport's size again
				// update viewport size
				//m_viewport->SetPosition({ 0,0 });
				if (updateViewportSize)
				{
					m_viewport->SetSize(viewportSize);
				}
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

		// callback when scrollview's size changes. we need to update the layout of the viewport and scrollbars
		void ScrollView::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			// remember viewport's size
			SizeF vp = m_viewport->GetSize();

			//when scrollview resize, the viewport component must resize too in order to fit in it
			//because viewport is resized, it might also need to reposition its content.
			//if content need to be scrolled as viewport gets bigger, content will be repositioned
			//this will raise content's 'Move' event. viewport will then try to reposition the content again for recorrecting if needed.
			//this is fine because if content does not need recorrecting, it will not reposition it anymore, preventing stack overflow.
			//the issue is, viewport will raise 'Scroll' event and ScrollView will be notified.
			//in ScrollView's viewport 'Scroll' event handler, it will update thumb position of its scrollbars.
			//when scrollbars' thumb position are updated (moved), it will raise its 'Scroll' event.
			//ScrollView is also subscribed to this, and its handler will set the ViewPort's offset. 
			//This will result in recursive loop between ViewPort and Scrollbar setting each other's offset!
			//to fix this, either ViewPort or Scrollbar must be the initiator of the offset update. the other one should just follow.
			//Since ViewPort holds the content, it is the initiator and scrollbars just sync into it.
			//So, in ScrollView's 'Resize' handler, we set a flag that when true, bypasses scrollbar's 'Scroll' event handler.
			m_ignoreScrollBarScroll = true;

			// because our size changed, we need to resize viewport to maintain its layout to fit in it
			// we also need to check if scrollbars are needed. 
			// this will show scrollbars if are needed. hide them if not.
			// viewport will also fit its size if scrollbars are hidden or visible
			UpdateComponentLayout();

			// if viewport resizes, scrollbar states must change too. its thumb sizes must change because they are in sync with content/viewport size
			// offset can change too so must update scrollbar offsets as well
			// but updating scrollbars should not trigger update to viewport. we already updated viewport.
			// viewport has its own mechanism to adjust its offset based on its viewport and content size.
			// scrollbars just syncs to it.
			if (vp == m_viewport->GetSize()) return;

			// update scrollbars' viewport size since viewport resized
			UpdateScrollBarViewportSize();

			// update scrollbars' thumb position
			UpdateThumbPositions();

			// allow receiving scrollbar scroll event notification
			m_ignoreScrollBarScroll = false;
		}

		// update the layout by resizing, repositioning the component layouts - viewport and scrollbars
		void ScrollView::UpdateComponentLayout()
		{			
			if (m_autoHideScrollBars)
			{
				// let's assume scroll bars are not needed first
				bool hScrollBarVisible = false;
				bool vScrollBarVisible = false;

				// since we assume there are no scrollbars so we also assume viewport occupies the whole scrollview
				SizeF viewportSize = GetSize();

				// if content.size < viewport.size, no need to do anything since scrollbars are hidden
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

				// now let's hide or show vertical scrollbar. do this only if the visibility of the scrollbar has changed. 
				if (vScrollBarVisible && !m_vScrollBar->IsVisible())
				{
					m_vScrollBar->Show();
				}
				else if (!vScrollBarVisible && m_vScrollBar->IsVisible())
				{
					m_vScrollBar->Hide();
				}

				// now let's hide or show horizontal scrollbar. do this only if the visibility of the scrollbar has changed. 
				if (hScrollBarVisible && !m_hScrollBar->IsVisible())
				{
					m_hScrollBar->Show();
				}
				else if (!hScrollBarVisible && m_hScrollBar->IsVisible())
				{
					m_hScrollBar->Hide();
				}

				// at this point, we already have the viewport size but have yet to set it. 
				// we also have scrollbars enabled if they are needed, disabled if not.

				// set the viewport size. SetSize() is smart enough to not raise 'Resize' event if we're setting it with the same size as before
				// so no event notification happens if no change in size
				m_viewport->SetSize(viewportSize);
			}
			// if scrollbars are not autohide, they are always visible
			else
			{
				// if scrollbars are always visible...
				if(!m_vScrollBar->IsVisible()) m_vScrollBar->Show();
				if(!m_hScrollBar->IsVisible()) m_hScrollBar->Show();

				// viewport top-left is always at scrollview's origin
				m_viewport->SetPosition({ 0,0 });

				// viewport always fills the scrollview extent minus the scrollbars which are always visible
				// we don't listen to viewport's resize event so nothing to do here.
				m_viewport->SetSize({
						GetSize().width - m_borderSize - m_scrollSize,
						GetSize().height - m_borderSize - m_scrollSize
					});
			}

			// we also update scrollbars' position and size. 
			// we don't listen scrollbars' 'Moved' and 'Resize' events so no risk of recursive deadlock here

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

			// we done here. it is not this method's responsibility to update components' logic states

			//UpdateScrollBarContentSize();
			//UpdateScrollBarViewportSize();
		}

		ScrollView::ScrollView(bool autoHideScrollBars, float scrollSize, float borderSize)
			: m_scrollSize(scrollSize)
			, m_borderSize(borderSize)
			, m_viewport(nullptr)
			, m_hScrollBar(nullptr)
			, m_vScrollBar(nullptr)
			, m_autoHideScrollBars(autoHideScrollBars)
		{
			// this widget is not draggable
			m_moveBehavior = MoveBehavior::None;

			// create viewport widget component
			std::unique_ptr<ViewPort> viewport = std::make_unique<ViewPort>();
			m_viewport = viewport.get();
			AddChild(std::move(viewport));

			// create horizontal scrollbar widget component
			std::unique_ptr<ScrollBar> hScrollBar = std::make_unique<ScrollBar>(0.0f, 0.0f, true);
			m_hScrollBar = hScrollBar.get();
			AddChild(std::move(hScrollBar));

			// create vertical scrollbar widget component
			std::unique_ptr<ScrollBar> vScrollBar = std::make_unique<ScrollBar>(0.f, 0.0f, false);
			m_vScrollBar = vScrollBar.get();
			AddChild(std::move(vScrollBar));

			// listen to viewport's scroll event. when viewport's content moves, we need to update our scrollbar's thumb positions
			m_viewport->Scroll += [&](const VecF& offset)
				{
					// update thumb positions of scrollbars to sync with viewport's content position
					// scrollbars should not trigger viewport to sync with it after their thumbs change position. otherwise it's gonna be deadlock
					// then finally raise Scroll event					


					if (!m_ignoreViewPortScroll) 

					// this will update thumb positions of the scroll bars
					UpdateThumbPositions();

					Scroll(offset);
				};

			m_viewport->ContentSizeChanged += [&](const SizeF& size)
				{
					// when content size changes, we first need to check if we need scrollbars. if it's autohide and we don't need it, we need to hide them
					// if scrollbars are active, we need to update size of the scrollbars' thumb
					// we also need to check if thumb position need to change. 
					// all these changes in scrollbar should not trigger viewport to sync with the scrollbar. otherwise it's gonna be deadlock

					
					// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
					//UpdateScrollBarContentSize();
					//UpdateLayout();

					// with new content, we need to check if scrollbars are needed. 
					// this will show scrollbars if are needed. hide them if not.
					// this will also resize the viewport to always fit in the scrollview
					UpdateComponentLayout();

					// new content might have a different size, offset or position relative to its viewport so we need to update scrollbar states
					// but since the viewport initiated the change here, we don't need to update the viewport after updating scrollbar thumbs
					m_ignoreScrollBarScroll = true;

					// update scrollbars' content size since we changed content
					UpdateScrollBarContentSize();

					// update scrollbars' thumb position
					UpdateThumbPositions();

					// allow receiving scrollbar scroll event notification
					m_ignoreScrollBarScroll = false;
				};

			m_viewport->ContentChanged += [&](Widget* content)
				{
					// when content changes, we need to check if we need scrollbars. if it's autohide and we don't need it, we need to hide them
					// if scrollbars are active, we need to update size of the scrollbars' thumb
					// we also need to check if thumb position need to change. 
					// all these changes in scrollbar should not trigger viewport to sync with the scrollbar. otherwise it's gonna be deadlock
					// then finally raise ContentChange event

					//// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
					//UpdateScrollBarContentSize();
					//UpdateLayout();


					// with new content, we need to check if scrollbars are needed. 
					// this will show scrollbars if are needed. hide them if not.
					// this will also resize the viewport to always fit in the scrollview
					UpdateComponentLayout();

					// new content might have a different size, offset or position relative to its viewport so we need to update scrollbar states
					// but since the viewport initiated the change here, we don't need to update the viewport after updating scrollbar thumbs
					m_ignoreScrollBarScroll = true;

					// update scrollbars' content size since we changed content
					UpdateScrollBarContentSize();

					// update scrollbars' thumb position
					UpdateThumbPositions();

					// allow receiving scrollbar scroll event notification
					m_ignoreScrollBarScroll = false;

					// bubble up the event to our own OnContentChange event
					ContentChanged(content);
				};

			// bubble up viewport's resize event to our own OnViewPortResize event. 
			// this is useful for external content that needs to resize itself when viewport resizes.
			m_viewport->Resized += [&](const SizeF& size)
				{
					// by right, viewport component does not resize by itself. we own this component. we decide when it resizes.
					// therefore when its 'Resized' event is raised, we don't have to do anything. 
					// TODO: we only emit this event to bubble it up. but do we really need to? let's check this 

					ViewPortResized(size);
				};

			// handler for when horizontal scrollbar scrolls. only viewport's content position changes here. layout remains the same.
			m_hScrollBar->Scroll += [&](float offset)
				{
					if (m_ignoreScrollBarScroll) return;

					VecF currOffset = m_viewport->GetOffset();
					currOffset.x = offset;

					m_ignoreViewPortScroll = true;
					m_viewport->SetOffset(currOffset);
					m_ignoreViewPortScroll = false;

					Scroll(currOffset);

				};

			// handler for when vertical scrollbar scrolls. only viewport's content position changes here. layout remains the same.
			m_vScrollBar->Scroll += [&](float offset)
				{
					if (m_ignoreScrollBarScroll) return;

					VecF currOffset = m_viewport->GetOffset();
					currOffset.y = offset;

					m_ignoreViewPortScroll = true;
					m_viewport->SetOffset(currOffset);
					m_ignoreViewPortScroll = false;

					Scroll(currOffset);
				};
		}

		void ScrollView::SetContentSize(const SizeF& size)
		{
			m_viewport->SetContentSize(size);
		}

		void ScrollView::SetContent(std::unique_ptr<Widget> content)
		{
			// first, set the content into viewport. this will update viewport with new content, new content size, and new offset. 
			// it is possible for viewport to raise its Scroll event if content needs to be repositioned to be within the bounds of viewport
			// but it is guaranteed that we will update thumb positions in 'ContentChanged' event handler

			m_viewport->SetContent(std::move(content));


			// with new content, we need to check if scrollbars are needed. 
			// we also need to update position of scrollbars' thumb based on current position of content
			// since we may need to enable or disable scrollbars, we may need to update viewport size. 

			//// with new content, we need to check if scrollbars are needed. 
			//// this will show scrollbars if are needed. hide them if not.
			//// if scrollbars change visibility state, viewport will be resized
			//UpdateComponentLayout();

			//// new content might have a different size, offset or position relative to its viewport so we need to update scrollbar states
			//// but since the viewport initiated the change here, we don't need to update the viewport after updating scrollbar thumbs
			//m_ignoreScrollBarScroll = true;

			//// update scrollbars' content size since we changed content
			//UpdateScrollBarContentSize();

			//// update scrollbars' thumb position
			//UpdateThumbPositions();

			//// allow receiving scrollbar scroll event notification
			//m_ignoreScrollBarScroll = false;
		}

		void ScrollView::SetContentPosition(const PositionF& pos)
		{
			m_viewport->SetOffset(PositionF{ 0,0 } - pos);
		}

		SizeF ScrollView::GetViewPortSize() const
		{
			return m_viewport->GetSize();
		}

		VecF ScrollView::GetViewPortOffset() const
		{
			return m_viewport->GetOffset();
		}

		void ScrollView::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawScrollView(*this, context);
		}
#pragma endregion
	}

}