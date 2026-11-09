#include <GUI/Controls.h>

#include <memory>
#include <algorithm>
#include <Utilities/Utilities.h>
#include <Utilities/Logger.h>


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
			case VerticalAnchor::Center:
				m_textPosition.y = (GetSize().height - m_textSize.height) / 2.0f;
				break;
			case VerticalAnchor::Top:
				m_textPosition.y = 0;
				break;
			case VerticalAnchor::Bottom:
				m_textPosition.y = GetSize().height - m_textSize.height;
				break;
			default:
				break;
			}

			switch (m_hAlign)
			{
			case HorizontalAnchor::Center:
				m_textPosition.x = (GetSize().width - m_textSize.width) / 2.0f;
				break;
			case HorizontalAnchor::Left:
				m_textPosition.x = 0;
				break;
			case HorizontalAnchor::Right:
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
			, m_vAlign(VerticalAnchor::Center)
			, m_hAlign(HorizontalAnchor::Center)
			, m_textSize({ 0,0 })
			, m_textPosition({ 0,0 })
			, m_color({ 1,1,1,1 })
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

		void Label::SetColor(const ColorF& color)
		{
			m_color = color;
		}

		ColorF Label::GetColor() const
		{
			return m_color;
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
		void Label::SetAlignment(VerticalAnchor vAlign, HorizontalAnchor hAlign)
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
		// constructor for Renderable as image resource
		Image::Image(Renderable renderable)
				: m_image(std::make_unique<Renderable>(std::move(renderable)))
				, m_hImageAnchor(HorizontalAnchor::Center)
				, m_vImageAnchor(VerticalAnchor::Center)
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		// constructor for Animated as image resource
		Image::Image(Animated animated)
				: m_image(std::make_unique<Animated>(std::move(animated)))
				, m_hImageAnchor(HorizontalAnchor::Center)
				, m_vImageAnchor(VerticalAnchor::Center)
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		void Image::Set(Renderable renderable)
		{
			m_image = std::make_unique<Renderable>(std::move(renderable));
		}

		void Image::Set(Animated animated)
		{
			m_image = std::make_unique<Animated>(std::move(animated));
		}

		// get the image's absolute position, accountin image's sprite pivot 
		PositionF Image::GetImageAbsolutePosition() const
		{
			return GetAbsolutePosition() + GetImagePosition();
		}

		// get the image's position relative to the widget's coordinates, accountin image's sprite pivot 
		PositionF Image::GetImagePosition() const
		{
			// set this as default position
			PositionF pos(0, 0);

			// why do we care about image's sprite pivot? because renderer will take sprite's pivot into account.
			// it will translate sprite's top-left position with its given pivot.
			// pivot is part of sprite's geometric description. Renderer must always be aware of it and take it into account when rendering sprite
			Sprite sprite = m_image->GetSprite();
			PositionF pivot = sprite.GetPivot();
			PositionF pivotInPixels = sprite.GetPivotInPixels();

			switch (m_vImageAnchor)
			{
			case VerticalAnchor::Center:
				pos.y = (GetSize().height - m_image->GetSprite().GetHeight()) / 2.0f + pivotInPixels.y;
				break;
			case VerticalAnchor::Top:
				pos.y = 0 + pivotInPixels.y;
				break;
			case VerticalAnchor::Bottom:
				pos.y = GetSize().height - m_image->GetSprite().GetHeight() + pivotInPixels.y;
				break;
			case VerticalAnchor::Fill:
				pos.y = 0 + pivot.y * GetHeight();
				break;
			default:
				throw std::runtime_error("Image::GetImagePosition(): invalid vertical anchor");
			}

			switch (m_hImageAnchor)
			{
			case HorizontalAnchor::Center:
				pos.x = (GetSize().width - m_image->GetSprite().GetWidth()) / 2.0f + pivotInPixels.x;
				break;
			case HorizontalAnchor::Left:
				pos.x = 0 + pivotInPixels.x;
				break;
			case HorizontalAnchor::Right:
				pos.x = GetSize().width - m_image->GetSprite().GetWidth() + pivotInPixels.x;
				break;
			case HorizontalAnchor::Fill:
				pos.x = 0 + pivot.x * GetWidth();
				break;
			default:
				throw std::runtime_error("Image::GetImagePosition(): invalid horizontal anchor");
			}
			return pos;
		}

		// get image's size based on its anchor type
		SizeF Image::GetImageSize() const
		{
			// set image' current sprite size as default
			SizeF size = m_image->GetSprite().GetSize();

			switch (m_vImageAnchor)
			{
			case VerticalAnchor::Center:
			case VerticalAnchor::Top:
			case VerticalAnchor::Bottom:
				// do nothing. we already set image's sprite height as default
				break;
			case VerticalAnchor::Fill:
				size.height = GetSize().height;
				break;
			default:
				throw std::runtime_error("Image::GetImageSize(): invalid vertical anchor");
			}

			switch (m_hImageAnchor)
			{
			case HorizontalAnchor::Center:
			case HorizontalAnchor::Left:
			case HorizontalAnchor::Right:
				// do nothing. we already set image's sprite width as default
				break;
			case HorizontalAnchor::Fill:
				size.width = GetSize().width;
				break;
			default:
				throw std::runtime_error("Image::GetImageSize(): invalid horizontal anchor");
			}

			return size;
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

		void Image::SetHorizontalImageAnchor(HorizontalAnchor hAnchor)
		{
			m_hImageAnchor = hAnchor;
		}

		void Image::SetVerticalImageAnchor(VerticalAnchor vAnchor)
		{
			m_vImageAnchor = vAnchor;
		}
#pragma endregion

#pragma region // Frame
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

		Thumb::Thumb(engine::gui::Orientation isHorizontal)
		{
			m_moveBehavior = isHorizontal == engine::gui::Orientation::Horizontal ? MoveBehavior::Horizontal : MoveBehavior::Vertical;
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
			if (m_orientation == Orientation::Horizontal)
			{
				float x = (GetSize().width - thumbSize.width) * nvalue;
				m_thumb->SetPosition({ x, 0 });
			}

			// vertical orientation
			else if (m_orientation == Orientation::Vertical)
			{
				float y = (GetSize().height - thumbSize.height) * nvalue;
				m_thumb->SetPosition({ 0, y });
			}
		}

		void Slider::UpdateThumbSize()
		{
			// if horizontal orientation, get slider height. otherwise, get width
			float thickness = m_orientation == Orientation::Horizontal ? GetHeight() : GetWidth();

			// get the slider length. 
			float sliderLength = m_orientation == Orientation::Horizontal ? GetWidth() : GetHeight();

			// if clamp thumb length within slider length, if needed
			float length = m_thumbLength < sliderLength ? m_thumbLength : sliderLength;

			SizeF thumbSize
			{
				m_orientation == Orientation::Horizontal ? length : thickness,
				m_orientation == Orientation::Horizontal ? thickness : length,
			};

			m_thumb->SetSize(thumbSize);
		}

		void Slider::UpdateValueFromPosition(const PositionF& position)
		{
			// translate the clicked position (world position) to slider's local coordinate
			PositionF local = position - GetAbsolutePosition();

			float thumbLength = m_orientation == Orientation::Horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;

			// get length of the slider. this is the length it can move, so subtract thumb length
			float length = m_orientation == Orientation::Horizontal ? GetSize().width - m_thumb->GetSize().width : GetSize().height - m_thumb->GetSize().height;

			// normalize the value of the position based on slider length
			float nvalue = length <= 0.0f ? 0.0f : (m_orientation == Orientation::Horizontal ? local.x - thumbLength / 2.0f : local.y - thumbLength / 2.0f) / length;

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
			if (m_range <= 1) return m_min;

			int steps = m_range - 1;

			float stepSize = (m_max - m_min) / steps;

			value = std::round((value - m_min) / stepSize);
			value *= stepSize;
			value += m_min;

			return value;
		}

		Slider::Slider(float min, float max, float thumbLength, int range, Orientation orientation) :
			m_min(min),
			m_max(max),
			m_thumbLength(thumbLength),
			m_value(min),
			m_orientation(orientation),
			m_isDragging(false),
			m_range(range)
		{
			m_moveBehavior = MoveBehavior::None;

			std::unique_ptr<SliderThumb> thumb = std::make_unique<SliderThumb>(m_orientation);
			m_thumb = thumb.get();

			AddChild(std::move(thumb));
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

		void Slider::SetRange(int range)
		{
			m_range = range;

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
			float lpos = (m_orientation == engine::gui::Orientation::Horizontal ? pos.x : pos.y) - (m_orientation == engine::gui::Orientation::Horizontal ? GetAbsolutePosition().x : GetAbsolutePosition().y);

			// get the size of the thumb. this is the length of the thumb widget along scrollbar's orientation
			float thumbSize = m_orientation == engine::gui::Orientation::Horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;

			// get the thumb position (local to scrollbar). this is the top position of thumb widget if scrollbar orientation is vertical, and left position if horizontal
			float thumbPos = m_orientation == engine::gui::Orientation::Horizontal ? m_thumb->GetPosition().x : m_thumb->GetPosition().y;

			// did we clicked on thumb or track?
			if (lpos < thumbPos || lpos > thumbPos + thumbSize)
			{
				// if click on track, move thumb to that position, and start dragging. the thumb will be centered on the click position if possible.
				float newThumbPos = lpos - thumbSize / 2.0f;

				// the new thumb pos could position thumb outside scrollbar's tracklength as the 'pos' from OnMouseDown could be outside scrollbar's area
				// that's fine. we just set it. we will validate the thumb position with our handler to thumb's 'Moved' event
				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPos,  m_thumb->GetPosition().y } : PositionF{ m_thumb->GetPosition().x, newThumbPos });
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
				// thumb is intentionally non-interactive. so to make it move, we have to do it ourselves by calling its mouse move
				m_thumb->MouseMove(pos);
			}
		}

		// internal method to update the size of the thumb based on the content length, viewport length, and scrollbar length. this is called when content or viewport changes.
		void ScrollBar::UpdateThumbSize()
		{
			// get the length of scrollbar based on its orientation.
			float trackLength = m_orientation == engine::gui::Orientation::Horizontal ? GetWidth() : GetHeight();
			float thickness = m_orientation == engine::gui::Orientation::Horizontal ? GetHeight() : GetWidth();

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
				m_orientation == engine::gui::Orientation::Horizontal ? length : thickness,
				m_orientation == engine::gui::Orientation::Horizontal ? thickness : length
			};
			m_thumb->SetSize(thumbSize);
		}	

		// internal method to convert logical position to physical position
		float ScrollBar::LogicalToPhysicalPosition(float logicalPos) const
		{
			// trackLength is the physical range the thumb can move within the scrollbar. this must be length of scrollbar minus length of thumb. 
			float trackLength = (m_orientation == engine::gui::Orientation::Horizontal ? GetSize().width : GetSize().height) 
				- (m_orientation == engine::gui::Orientation::Horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height);

			// this is the logical range in which viewport can move. 
			float logicalRange = m_contentLength - m_viewportLength;

			// we can have negative logical range, but if that's the case, it must be clamped to 0. 
			// so if viewport length is greater than content length, logical range is < 0. scrolling is NOT required
			logicalRange = std::max<float>(0.0f, logicalRange);

			// we can't have negative track length. if thumb is same size as scrollbar, scrolling is NOT required
			trackLength = std::max<float>(0.0f, trackLength);

			// now let's calculate the thumb's physical position by translating its logical position
			// note that if logical range is <= 0, thumb is not scrolling. therefore it will be set to 0
			return logicalRange == 0 ? 0.0f : logicalPos * trackLength / logicalRange;
		}

		// internal method to convert physical position to logical position
		float ScrollBar::PhysicalToLogicalPosition(float physicalPos) const
		{
			// trackLength is the physical range the thumb can move within the scrollbar. this must be length of scrollbar minus length of thumb. 
			float trackLength = (m_orientation == engine::gui::Orientation::Horizontal ? GetSize().width : GetSize().height) 
				- (m_orientation == engine::gui::Orientation::Horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height);

			// this is the logical range in which viewport can move. 
			float logicalRange = m_contentLength - m_viewportLength;

			// we can have negative logical range, but if that's the case, it must be clamped to 0. 
			// so if viewport length is greater than content length, logical range is < 0. scrolling is NOT required
			logicalRange = std::max<float>(0.0f, logicalRange);

			// we can't have negative track length. if thumb is same size as scrollbar, scrolling is NOT required
			trackLength = std::max<float>(0.0f, trackLength);

			// now let's calculate the thumb's logical position by translating its physical position
			// note that if tracklength is <= 0, thumb is not scrolling. therefore it will be set to 0
			return trackLength == 0 ? 0.0f : physicalPos * logicalRange / trackLength;
		}

		// internal method to round off the given value to nearest step size
		float ScrollBar::SnapToNearestStep(float value, float step) const
		{
			// sanity check. if step is invalid, just return the value
			if (step <= 0) return value;

			// a more descriptive implementation
			//return std::floor((value + step * 0.5f) / step) * step;
			
			// simplified version
			return std::floor(value / step + 0.5f) * step;
		}

		// callback when the size of the scrollbar changes. 
		// note that logical parameters (offset, viewport/content length) does not change. only physical size change
		void ScrollBar::OnSizeChanged(const SizeF&, const SizeF&)
		{
			LOG_FUNCTION();

			// logical parameters did not changed but scrollbar size did. 
			// to maintain proportion between logical (content/viewport length) and physical (scrollbar/thumb size), we need to update thumb size
			UpdateThumbSize();

			// offset did not changed. but since thumb and scrollbar size changed, thumb might have a different position now to match offset. convert offset to physical position.
			float thumbPhysicalPos = LogicalToPhysicalPosition(m_offset);

			// we then set thumb to new physical position. we ignore the move event handler since it's guaranteed that the new thumb position is valid
			{
				// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
				m_ignoreThumbMove = true;

				// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
				engine::utilities::OnOutOfScope cleanup([&]
					{
						m_ignoreThumbMove = false;
					});

				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ thumbPhysicalPos, 0 } : PositionF{ 0, thumbPhysicalPos });
			}
		}

		// constructor
		ScrollBar::ScrollBar(float contentLength, float viewportLength, engine::gui::Orientation orientation, float minThumLength, float stepSize, ScrollBar::ScrollMode mode)
			: m_contentLength(contentLength)
			, m_viewportLength(viewportLength)
			, m_offset(0)
			, m_orientation(orientation)
			, m_isDragging(false)
			, m_minThumbLength(minThumLength)
			, m_stepSize(stepSize)
			, m_scrollMode(mode)
		{
			// this scrollbar is not movable. it is a static widget that can only be moved by dragging the thumb
			m_moveBehavior = MoveBehavior::None;

			// create thumb and add it as child. we will use this thumb to handle dragging and scrolling
			std::unique_ptr<ScrollThumb> thumb = std::make_unique<ScrollThumb>(m_orientation);
			m_thumb = thumb.get();
			AddChild(std::move(thumb));

			// handle thumb movement. when thumb is moved, we will calculate the new offset based on thumb position and content length, viewport length, and scrollbar length
			// the purpose of this handler is to recorrect the thumb position to make sure it is within the scrollbar's scroll range
			m_thumb->Moved += [this](const PositionF& oldPos, const PositionF& newPos)
				{
					// bail out if ignoring thumb move
					if (m_ignoreThumbMove) return;

					// get the thumb's physical position
					float thumbPhysicalPos = m_orientation == engine::gui::Orientation::Horizontal ? newPos.x : newPos.y;

					// convert thumb's physical position to logical position. this will be clamped to logical range
					float thumbLogicalPos = PhysicalToLogicalPosition(thumbPhysicalPos);

					// calculate logical range and clamp to >= 0. we will use this later
					float logicalRange = std::max<float>(0.0f, m_contentLength - m_viewportLength);

					// are we discrete scrolling? if yes, snap to nearest. 
					if (m_scrollMode == ScrollMode::Discrete)
					{
						// calculate logical position that snaps to step size and is nearest to current offset
						float snapped = SnapToNearestStep(thumbLogicalPos, m_stepSize);

						// this is special handling on use case where current offset is at maximum boundary. 
						// if say step size = 1.5 and logical range is [0, 5], the steps are [0, 1.5, 3.0, 4.5, 5]
						// if say current offset = 3, then you set it to 5 as new offset, the result will snap to 4.5 when it should be 5
						// to fix this, check which is nearer to the current offset, max boundary? or the snapped 4.5?
						thumbLogicalPos = (logicalRange - thumbLogicalPos <= thumbLogicalPos - snapped) ? logicalRange : snapped;
					}

					// clamp logical pos to be within logical range [0, logical range]
					thumbLogicalPos = std::clamp(thumbLogicalPos, 0.0f, logicalRange);

					// now we have valid logical position. convert to physical position. this will be clamped to trackLength
					float newThumbPhysicalPos = LogicalToPhysicalPosition(thumbLogicalPos);

					// we then set thumb to new physical position. we ignore the move event handler since we guaranteed that the new thumb position is valid, no need for correction
					{
						// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
						m_ignoreThumbMove = true;

						// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
						engine::utilities::OnOutOfScope cleanup([&]
							{
								m_ignoreThumbMove = false;
							});

						m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPhysicalPos, 0 } : PositionF{ 0, newThumbPhysicalPos });
					}

					// if logical position is not same as current offset, set logical position as new offset. then raise Scroll event
					if (m_offset == thumbLogicalPos) return;
					m_offset = thumbLogicalPos;
					if(IsVerbose()) LOG("m_thumb->Moved() offset: " + std::to_string(m_offset));
					Scroll(m_offset);
				};
		}

		// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset actually changed.
		void ScrollBar::SetOffset(float offset)
		{
			LOG_FUNCTION();

			// remember old offset as we will refer to this later to check if we indeed changed offset after validating it
			float oldOffset = m_offset;

			// set new offset
			m_offset = offset;

			// calculate logical range and clamp to >= 0. we will use this later
			float logicalRange = std::max<float>(0.0f, m_contentLength - m_viewportLength);

			// but wait, if scroll mode is discrete, offset must snap to nearest step size.
			if (m_scrollMode == ScrollMode::Discrete)
			{
				// calculate logical position that snaps to step size and is nearest to current offset
				float snapped = SnapToNearestStep(m_offset, m_stepSize);

				// this is special handling on use case where current offset is at maximum boundary. 
				// if say step size = 1.5 and logical range is [0, 5], the steps are [0, 1.5, 3.0, 4.5, 5]
				// if say current offset = 3, then you set it to 5 as new offset, the result will snap to 4.5 when it should be 5
				// to fix this, check which is nearer to the current offset, max boundary? or the snapped 4.5?
				m_offset = (logicalRange - m_offset <= m_offset - snapped) ? logicalRange : snapped;
			}

			// clamp logical pos to be within logical range [0, logical range]
			m_offset = std::clamp(m_offset, 0.0f, logicalRange);

			// now we have valid logical position. convert to physical position. this will be clamped to trackLength
			float newThumbPhysicalPos = LogicalToPhysicalPosition(m_offset);

			// if new physical position is not same as current thumb position, we set position again. but this time, ignore thumb move event. 
			{
				// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
				m_ignoreThumbMove = true;

				// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
				engine::utilities::OnOutOfScope cleanup([&]
					{
						m_ignoreThumbMove = false;
					});

				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPhysicalPos, 0 } : PositionF{ 0, newThumbPhysicalPos });
			}

			// finally, if the resulting new offset is not same as old offset, we raise Scroll event
			if (oldOffset == m_offset) return;
			if (IsVerbose()) LOG("ScrollBar::SetOffset() offset: " + std::to_string(m_offset));
			Scroll(m_offset);
		}

		// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
		float ScrollBar::GetOffset() const
		{
			return m_offset;
		}

		// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
		void ScrollBar::SetContentLength(float length)
		{
			LOG_FUNCTION();

			// clamp the new length to be sure its valid. it cannot be negative.
			length = std::max<float>(0.0f, length);

			// bail out if new length is same as current content length
			if (length == m_contentLength) return;

			// let's now change to new length
			m_contentLength = length;

			// since content length changes, proportion with thumb and scrollbar length changes. update thumb size to maintain the proportion
			UpdateThumbSize();

			// content length changed. it's possible current offset is now outside logical range. so we check. if it is, we need to clamp it be within logical range
			// if scroll mode is discrete, offset is still definitely snapped so we don't have to snap it again
			float thumbLogicalPos = m_offset;

			// but we need to clamp it to new logical range if necessary [0, logical range]
			float logicalRange = std::max<float>(0.0f, m_contentLength - m_viewportLength);
			thumbLogicalPos = std::clamp(thumbLogicalPos, 0.0f, logicalRange);

			// now we have valid logical position (offset), let's convert to physical position. 
			// since our offset is valid, the resulting thumb physical position is guaranteed to be within track length
			float newThumbPhysicalPos = LogicalToPhysicalPosition(thumbLogicalPos);

			// we then set thumb to new physical position. we ignore the move event handler since we guaranteed that the new thumb position is valid, no need for correction
			{
				// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
				m_ignoreThumbMove = true;

				// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
				engine::utilities::OnOutOfScope cleanup([&]
					{
						m_ignoreThumbMove = false;
					});

				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPhysicalPos, 0 } : PositionF{ 0, newThumbPhysicalPos });
			}

			// if the new logical position is not same as previous offset, set logical position as new offset. then raise Scroll event
			if (m_offset == thumbLogicalPos) return;
			m_offset = thumbLogicalPos;
			if(IsVerbose()) LOG("ScrollBar::SetContentLength() offset: " + std::to_string(m_offset));
			Scroll(m_offset);
		}

		// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
		void ScrollBar::SetViewportLength(float length)
		{
			LOG_FUNCTION();

			// clamp the new length to be sure its valid. it cannot be negative.
			length = std::max<float>(0.0f, length);

			// bail out if new length is same as current viewport length
			if (length == m_viewportLength) return;

			// let's now change to new length
			m_viewportLength = length;

			// since viewport length changes, proportion with thumb and scrollbar length changes. update thumb size to maintain the proportion
			UpdateThumbSize();

			// viewport length changed. it's possible current offset is now outside logical range. so we check. if it is, we need to clamp it be within logical range
			// if scroll mode is discrete, offset is still definitely snapped so we don't have to snap it again
			float thumbLogicalPos = m_offset;

			// but we need to clamp it to new logical range if necessary [0, logical range]
			float logicalRange = std::max<float>(0.0f, m_contentLength - m_viewportLength);
			thumbLogicalPos = std::clamp(thumbLogicalPos, 0.0f, logicalRange);

			// now we have valid logical position (offset), let's convert to physical position. 
			// since our offset is valid, the resulting thumb physical position is guaranteed to be within track length
			float newThumbPhysicalPos = LogicalToPhysicalPosition(thumbLogicalPos);

			// we then set thumb to new physical position. we ignore the move event handler since we guaranteed that the new thumb position is valid, no need for correction
			{
				// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
				m_ignoreThumbMove = true;

				// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
				engine::utilities::OnOutOfScope cleanup([&]
					{
						m_ignoreThumbMove = false;
					});

				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPhysicalPos, 0 } : PositionF{ 0, newThumbPhysicalPos });
			}

			// if the new logical position is not same as previous offset, set logical position as new offset. then raise Scroll event
			if (m_offset == thumbLogicalPos) return;
			m_offset = thumbLogicalPos;
			if (IsVerbose())LOG("ScrollBar::SetViewportLength() offset: " + std::to_string(m_offset));
			Scroll(m_offset);
		}

		// set step size. if curren scrollmode is discrete, this will snap the thumb into nearest step
		void ScrollBar::SetStepSize(float step)
		{
			LOG_FUNCTION();

			// clamp the new step size to be sure its valid. it cannot be negative.
			step = std::max<float>(0.0f, step);

			// bail out if new step size is same as current step size
			if (step == m_stepSize) return;

			// let's now change step size
			m_stepSize = step;

			// note: it does not matter if step size is too big, even bigger than track length. thumb's move event handler will always tame the thumb position
			// to be within scroll range

			// remember old offset. in case we change it, we must raise Scroll event
			float oldOffset = m_offset;

			// calculate logical range and clamp to >= 0. we will use this later
			float logicalRange = std::max<float>(0.0f, m_contentLength - m_viewportLength);

			// if scroll mode is discrete, offset must snap to nearest step size since we changed step size
			if (m_scrollMode == ScrollMode::Discrete)
			{
				// calculate logical position that snaps to step size and is nearest to current offset
				float snapped = SnapToNearestStep(m_offset, m_stepSize);

				// this is special handling on use case where current offset is at maximum boundary. 
				// if say step size = 1.5 and logical range is [0, 5], the steps are [0, 1.5, 3.0, 4.5, 5]
				// if say current offset = 3, then you set it to 5 as new offset, the result will snap to 4.5 when it should be 5
				// to fix this, check which is nearer to the current offset, max boundary? or the snapped 4.5?
				m_offset = (logicalRange - m_offset <= m_offset - snapped) ? logicalRange : snapped;
			}
			// if we're not running on discrete step, no need to validate thumb position and offset
			else
			{
				return;
			}

			// clamp logical pos to be within logical range [0, logical range]
			m_offset = std::clamp(m_offset, 0.0f, logicalRange);

			// now we have valid logical position. convert to physical position. this will be clamped to trackLength
			float newThumbPhysicalPos = LogicalToPhysicalPosition(m_offset);

			// if new physical position is not same as current thumb position, we set position again. but this time, ignore thumb move event. 
			{
				// since the physical thumb position we are now setting thumb to is guaranteed to be valid, we can disable listening to thumb's Moved event when set the position again
				m_ignoreThumbMove = true;

				// RAII: this ensures m_ignoreThumbMove state is back to normal (set to false) after we go out of scope.
				engine::utilities::OnOutOfScope cleanup([&]
					{
						m_ignoreThumbMove = false;
					});

				m_thumb->SetPosition(m_orientation == engine::gui::Orientation::Horizontal ? PositionF{ newThumbPhysicalPos, 0 } : PositionF{ 0, newThumbPhysicalPos });
			}

			// finally, if the resulting new offset is not same as old offset, we raise Scroll event
			if (oldOffset == m_offset) return;
			if (IsVerbose())LOG("ScrollBar::SetStepSize() offset: " + std::to_string(m_offset));
			Scroll(m_offset);
		}

		float ScrollBar::GetStepSize() const
		{
			return m_stepSize;
		}

		// set scroll mode. if mode is now discrete, this will update the offset to snap to nearest step
		void ScrollBar::SetScrollMode(ScrollBar::ScrollMode mode)
		{
			LOG_FUNCTION();

			// if same mode, do nothing
			if (mode == m_scrollMode) return;

			// set to new mode
			m_scrollMode = mode;

			// if new mode is smooth scrolling, we don't have to do anything
			if (m_scrollMode == ScrollBar::ScrollMode::Smooth) return;

			// if new mode is discrete, then we may need to update thumb position so it will snap to step size if needed
			if (m_scrollMode == ScrollBar::ScrollMode::Discrete)
			{
				// we only need to update if scroll mode is discrete.
				// logical range remains the same. but offset might change since it needs to snap now. so let's set it
				SetOffset(m_offset);
			}
		}

		ScrollBar::ScrollMode ScrollBar::GetScrollMode() const
		{
			return m_scrollMode;
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
				m_topRightResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateTopRightGripLayout(); };
				m_topResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateTopGripLayout(); };
				m_topLeftResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateTopLeftGripLayout(); };
				m_rightResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateRightGripLayout(); };
				m_leftResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateLeftGripLayout(); };
				m_bottomRightResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateBottomRightGripLayout(); };
				m_bottomResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateBottomGripLayout(); };
				m_bottomLeftResizeGrip->Moved += [&](const PositionF& oldPos, const PositionF& newPos) { UpdateBottomLeftGripLayout(); };
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
	
		// this method ensures that content's position is always within the viewport's bounds. 
		// if content's position is outside the viewport, it will be moved back to the nearest position within the viewport
		bool ScrollView::ViewPort::ResolveContentPosition()
		{
			LOG_FUNCTION();

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
			LOG_FUNCTION();

			ResolveContentPosition();
		}

		// event handler for when content's size changes. we need to ensure that content's position is still within the viewport's bounds
		void ScrollView::ViewPort::OnContentSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			LOG_FUNCTION();

			// we need to ensure that the content is within the viewport's bounds after resizing
			ResolveContentPosition();

			// bubble up the event to notify that content's size has changed. this is useful for scroll bars to update their thumb size and position
			if (IsVerbose()) LOG("ScrollView::ViewPort::OnContentSizeChanged() - " + std::to_string(newSize.width) + ", " + std::to_string(newSize.height));

			ContentSizeChanged(newSize);
		}

		// callback when content change position. 
		void ScrollView::ViewPort::OnContentMove(const PositionF& oldPos, const PositionF& newPos)
		{
			LOG_FUNCTION();

			// try to resolve content's position. if it is outside the viewport, it will be moved back to the nearest position within the viewport
			ResolveContentPosition();

			// if content's position is still the same as before after recorrection (if needed), then we don't need to do anything. 
			// we only need to fire scroll event if content's position has changed
			if(oldPos == m_content->GetPosition())
			{
				return;
			}

			// fire scroll event to notify that content has moved and viewport's offset has changed
			if (IsVerbose()) LOG("ScrollView::ViewPort::OnContentMove() " + std::to_string(GetOffset().x) + ", " + std::to_string(GetOffset().y));
			Scroll(GetOffset());
		}

		ScrollView::ViewPort::ViewPort()
			:m_content(nullptr)
		{
			LOG_FUNCTION();

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

		Widget* ScrollView::ViewPort::GetContent() const
		{
			return m_content;
		}

		void ScrollView::ViewPort::SetContent(std::unique_ptr<Widget> content)
		{
			LOG_FUNCTION();

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
			ResolveContentPosition();

			// fire event to notify that content has changed
			if (IsVerbose()) LOG("ScrollView::ViewPort::SetContent() - Content has changed");
			ContentChanged(m_content);
		}

		void ScrollView::ViewPort::SetContentSize(const SizeF& size)
		{
			LOG_FUNCTION();

			m_content->SetSize(size);
		}

		void ScrollView::ViewPort::SetOffset(const VecF& offset)
		{
			LOG_FUNCTION();

			// setting the viewport offset is done by moving the content in the opposite direction relative to the viewport. this ultimately calls m_content->SetPosition().
			// if position actually changes, it will raise content's 'Moved' event. position will be recorrected if it is outside the viewport's bounds. 
			PositionF pos = PositionF{ 0,0 } - offset;
			m_content->SetPosition(pos);
		}
#pragma endregion

#pragma region // ScrollView
		// must be called when viewport's offset changes, e.g. when content is moved.
		void ScrollView::UpdateScrollBarOffsets()
		{
			LOG_FUNCTION();

			// when content moves, viewport's offset changes. 
			VecF offset = m_viewport->GetOffset();

			// we need to update the scrollbars' offset to reflect the new offset of the viewport
			if (m_hScrollBar->GetOffset() != offset.x) m_hScrollBar->SetOffset(offset.x);
			if (m_vScrollBar->GetOffset() != offset.y) m_vScrollBar->SetOffset(offset.y);
		}

		// this will update the content size of scroll bars. scroll bars then will refresh its thumb size and thumb position internally
		void ScrollView::UpdateScrollBarContentSize()
		{
			LOG_FUNCTION();

			SizeF contentSize = m_viewport->GetContentSize();

			m_hScrollBar->SetContentLength(contentSize.width);
			m_vScrollBar->SetContentLength(contentSize.height);
		}

		// this will update the viewport size of scroll bars.  scroll bars then will refresh its thumb size and thumb position internally
		void ScrollView::UpdateScrollBarViewportSize()
		{
			LOG_FUNCTION();

			SizeF viewportSize = m_viewport->GetSize();

			m_hScrollBar->SetViewportLength(viewportSize.width);
			m_vScrollBar->SetViewportLength(viewportSize.height);
		}

		// update the layout by resizing, repositioning the component layouts - viewport and scrollbars
		void ScrollView::UpdateComponentLayout()
		{
			LOG_FUNCTION();

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
				if (!m_vScrollBar->IsVisible()) m_vScrollBar->Show();
				if (!m_hScrollBar->IsVisible()) m_hScrollBar->Show();

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
		}

		bool ScrollView::OnMouseWheel(const PositionF& screenPos, float delta)
		{
			//LOG("ScrollBar::OnMouseWheel(" + m_name + "): " + std::to_string(screenPos.x) + ", " + std::to_string(screenPos.y) + ", " + std::to_string(delta));

			float scrollAmount = m_vScrollBar->GetViewportLength() * m_mouseWheelScrollFactor;
			m_vScrollBar->SetOffset(m_vScrollBar->GetOffset() - delta * scrollAmount);

			// we processed this event and does not need to propagate it anymore
			return true;
		}

		bool ScrollView::OnMouseHWheel(const PositionF& screenPos, float delta)
		{
			//LOG("ScrollBar::OnMouseHWheel(" + m_name + "): " + std::to_string(screenPos.x) + ", " + std::to_string(screenPos.y) + ", " + std::to_string(delta));

			float scrollAmount = m_hScrollBar->GetViewportLength() * m_mouseWheelScrollFactor;
			m_hScrollBar->SetOffset(m_hScrollBar->GetOffset() + delta * scrollAmount);

			// we processed this event and does not need to propagate it anymore
			return true;
		}



		void ScrollView::OnVerbose(bool enable)
		{
			LOG_FUNCTION();

			m_vScrollBar->Verbose(enable);
			m_hScrollBar->Verbose(enable);
			m_viewport->Verbose(enable);
		}

		// callback when scrollview's size changes. we need to update the layout of the viewport and scrollbars
		void ScrollView::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			LOG_FUNCTION();

			// disable scrollbars' scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
			// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
			bool oldIgnoreScrollBarScroll = m_ignoreScrollBarScroll;
			bool oldIgnoreViewPortScroll = m_ignoreViewPortScroll;
			m_ignoreScrollBarScroll = true;
			m_ignoreViewPortScroll = true;

			// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
			engine::utilities::OnOutOfScope cleanup([&]
				{
					m_ignoreScrollBarScroll = oldIgnoreScrollBarScroll;
					m_ignoreViewPortScroll = oldIgnoreViewPortScroll;
				});

			// remember the viewport's offset before resizing. we will check if it changed after resizing
			VecF offset = m_viewport->GetOffset();
		
			// resize viewport, reposition and resize scrollbars to fit in scrollview's new size. 
			UpdateComponentLayout();

			// update scrollbars' viewport size since viewport resized
			UpdateScrollBarViewportSize();

			// check if scrollbars' current offsets are different from viewport's offset after resizing viewport. 
			// if different, update scrollbars' offset to match viewport's offset.
			UpdateScrollBarOffsets();

			// if offset changed, raise Scroll event
			if(offset != m_viewport->GetOffset())
			{
				if (IsVerbose()) LOG("ScrollView::OnSizeChanged() - offset: " + std::to_string(m_viewport->GetOffset().x) + ", " + std::to_string(m_viewport->GetOffset().y));
				Scroll(m_viewport->GetOffset());
			}
		}

		ScrollView::ScrollView(bool autoHideScrollBars, float scrollSize, float borderSize)
			: m_scrollSize(scrollSize)
			, m_borderSize(borderSize)
			, m_viewport(nullptr)
			, m_hScrollBar(nullptr)
			, m_vScrollBar(nullptr)
			, m_autoHideScrollBars(autoHideScrollBars)
		{
			LOG_FUNCTION();

			// this widget is not draggable
			m_moveBehavior = MoveBehavior::None;

			// create viewport widget component
			std::unique_ptr<ViewPort> viewport = std::make_unique<ViewPort>();
			viewport->SetName("viewport");
			m_viewport = viewport.get();
			AddChild(std::move(viewport));

			// create horizontal scrollbar widget component
			std::unique_ptr<ScrollBar> hScrollBar = std::make_unique<ScrollBar>(0.0f, 0.0f, engine::gui::Orientation::Horizontal);
			hScrollBar->SetName("hscrollbar");
			m_hScrollBar = hScrollBar.get();
			AddChild(std::move(hScrollBar));

			// create vertical scrollbar widget component
			std::unique_ptr<ScrollBar> vScrollBar = std::make_unique<ScrollBar>(0.f, 0.0f, engine::gui::Orientation::Vertical);
			vScrollBar->SetName("vscrollbar");
			m_vScrollBar = vScrollBar.get();
			AddChild(std::move(vScrollBar));

			// listen to viewport's scroll event. when viewport's content moves, we need to update our scrollbar's thumb positions
			m_viewport->Scroll += [&](const VecF& offset)
				{
					LOG_FUNCTION();
					
					// if we're ignoring this event, bail out. this is to prevent recursive loop between viewport and scrollbars setting each other's offset
					if (m_ignoreViewPortScroll) return;

					{
						// disable scrollbars' scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
						// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
						bool oldIgnoreScrollBarScroll = m_ignoreScrollBarScroll;
						m_ignoreScrollBarScroll = true;

						// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
						engine::utilities::OnOutOfScope cleanup([&]
							{
								m_ignoreScrollBarScroll = oldIgnoreScrollBarScroll;
							});

						// update scrollbars' offsets to match viewport's offset if they are different. this will update scrollbars' thumb position
						UpdateScrollBarOffsets();
					}

					if (IsVerbose()) LOG("ScrollView handler > ViewPort::Scroll " + std::to_string(offset.x) + ", " + std::to_string(offset.y));
					Scroll(offset);
				};

			m_viewport->ContentSizeChanged += [&](const SizeF& size)
				{
					LOG_FUNCTION();

					// disable scrollbars' scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
					// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
					bool oldIgnoreScrollBarScroll = m_ignoreScrollBarScroll;
					bool oldIgnoreViewPortScroll = m_ignoreViewPortScroll;
					m_ignoreScrollBarScroll = true;
					m_ignoreViewPortScroll = true;

					// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
					engine::utilities::OnOutOfScope cleanup([&]
						{
							m_ignoreScrollBarScroll = oldIgnoreScrollBarScroll;
							m_ignoreViewPortScroll = oldIgnoreViewPortScroll;
						});

					// when content size changes, we need to check if scrollbars are needed. if it's autohide and we don't need it, we need to hide them
					// this will resize viewport to fit in scrollview if scrollbars are hidden or shown
					UpdateComponentLayout();

					// content size changed so viewport size should not have changed. 
					// but viewport might have resized during UpdateComponentLayout() so we need to update scrollbars' viewport size
					UpdateScrollBarViewportSize();

					// update scrollbars' content size since we changed content size. this might change offset.
					UpdateScrollBarContentSize();

					// update scrollbars' offsets to match viewport's offset if they are different. this will update scrollbars' thumb position
					UpdateScrollBarOffsets();
				};

			m_viewport->ContentChanged += [&](Widget* content)
				{
					LOG_FUNCTION();

					// disable scrollbars' scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
					// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
					bool oldIgnoreScrollBarScroll = m_ignoreScrollBarScroll;
					bool oldIgnoreViewPortScroll = m_ignoreViewPortScroll;
					m_ignoreScrollBarScroll = true;
					m_ignoreViewPortScroll = true;

					// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
					engine::utilities::OnOutOfScope cleanup([&]
						{
							m_ignoreScrollBarScroll = oldIgnoreScrollBarScroll;
							m_ignoreViewPortScroll = oldIgnoreViewPortScroll;
						});

					// when content size changes, we need to check if scrollbars are needed. if it's autohide and we don't need it, we need to hide them
					// this will resize viewport to fit in scrollview if scrollbars are hidden or shown
					UpdateComponentLayout();

					// content size changed so viewport size should not have changed. 
					// but viewport might have resized during UpdateComponentLayout() so we need to update scrollbars' viewport size
					UpdateScrollBarViewportSize();

					// update scrollbars' content size since we changed content size. this might change offset.
					UpdateScrollBarContentSize();

					// update scrollbars' offsets to match viewport's offset if they are different. this will update scrollbars' thumb position
					UpdateScrollBarOffsets();

					// bubble up the event to our own OnContentChange event
					if (IsVerbose()) LOG("ScrollView handler > ViewPort::ContentChanged ");
					ContentChanged(content);
				};

			m_viewport->Resized += [&](const SizeF& oldSize, const SizeF& newSize)
				{
					// we don't really care for this event since we own viewport object and full control of its size
					// but we can raise event when it resizes so if application needs to handle when viewport resizes, they can be notified
					ViewPortResized(oldSize, newSize);
				};

			// handler for when horizontal scrollbar scrolls. only viewport's content position changes here. layout remains the same.
			m_hScrollBar->Scroll += [&](float offset)
				{
					LOG_FUNCTION();

					// if we're ignoring this event, bail out. this is to prevent recursive loop between viewport and scrollbars setting each other's offset
					if (m_ignoreScrollBarScroll) return;

					// get the offset of the viewport. we will only change the x offset since this is horizontal scrollbar
					VecF currOffset = m_viewport->GetOffset();
					currOffset.x = offset;

					{
						// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
						bool oldIgnoreViewPortScroll = m_ignoreViewPortScroll;
						m_ignoreViewPortScroll = true;

						// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
						engine::utilities::OnOutOfScope cleanup([&]
							{
								m_ignoreViewPortScroll = oldIgnoreViewPortScroll;
							});

						// we now set the viewport's offset to the new offset. this will move the content inside the viewport and raise viewport's Scroll event. but we'll ignore the event.
						m_viewport->SetOffset(currOffset);
					}

					// raise scroll event to notify content scrolled because horizontal scrollbar scrolled. 
					if (IsVerbose()) LOG("ScrollView handler > HScrollBar:Scroll " + std::to_string(currOffset.x) + ", " + std::to_string(currOffset.y));
					Scroll(currOffset);

				};

			// handler for when vertical scrollbar scrolls. only viewport's content position changes here. layout remains the same.
			m_vScrollBar->Scroll += [&](float offset)
				{
					LOG_FUNCTION();

					// if we're ignoring this event, bail out. this is to prevent recursive loop between viewport and scrollbars setting each other's offset
					if (m_ignoreScrollBarScroll) return;

					// get the offset of the viewport. we will only change the y offset since this is vertical scrollbar
					VecF currOffset = m_viewport->GetOffset();
					currOffset.y = offset;

					{
						// disable viewport's scroll event notification to prevent recursive loop between viewport and scrollbars setting each other's offset
						bool oldIgnoreViewPortScroll = m_ignoreViewPortScroll;
						m_ignoreViewPortScroll = true;

						// RAII: this ensures components' scroll event notification is restored back to its original state when this method exits, even if it exits due to exception
						engine::utilities::OnOutOfScope cleanup([&]
							{
								m_ignoreViewPortScroll = oldIgnoreViewPortScroll;
							});

						// we now set the viewport's offset to the new offset. this will move the content inside the viewport and raise viewport's Scroll event. but we'll ignore the event.
						m_viewport->SetOffset(currOffset);
					}

					// raise scroll event to notify content scrolled because vertical scrollbar scrolled. 
					if (IsVerbose()) LOG("ScrollView handler > VScrollBar:Scroll " + std::to_string(currOffset.x) + ", " + std::to_string(currOffset.y));
					Scroll(currOffset);
				};
		}

		void ScrollView::SetContentSize(const SizeF& size)
		{
			LOG_FUNCTION();

			m_viewport->SetContentSize(size);
		}

		void ScrollView::SetContent(std::unique_ptr<Widget> content)
		{
			LOG_FUNCTION();

			// first, set the content into viewport. this will update viewport with new content, new content size, and new offset. 
			// it is possible for viewport to raise its Scroll event if content needs to be repositioned to be within the bounds of viewport
			// but it is guaranteed that we will update thumb positions in 'ContentChanged' event handler

			m_viewport->SetContent(std::move(content));
		}

		void ScrollView::SetContentPosition(const PositionF& pos)
		{
			LOG_FUNCTION();

			m_viewport->SetOffset(PositionF{ 0,0 } - pos);
		}

		SizeF ScrollView::GetViewPortSize() const
		{
			LOG_FUNCTION();

			return m_viewport->GetSize();
		}

		VecF ScrollView::GetViewPortOffset() const
		{
			LOG_FUNCTION();

			return m_viewport->GetOffset();
		}

		void ScrollView::SetOffset(const VecF& offset)
		{
			LOG_FUNCTION();

			m_viewport->SetOffset(offset);
		}


		void ScrollView::Draw(const UIDrawContext& context) const
		{
			context.skin.DrawScrollView(*this, context);
		}
#pragma endregion
	}

}

namespace engine
{
	namespace gui
	{
#pragma region // TextItem
		// when this widget resizes, we need to resize the label to fill the whole area of this widget
		void TextItem::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			m_label->SetPosition({ 0.0f, 0.0f });
			m_label->SetSize(newSize);
		}

		// raise click event when this item is selected
		void TextItem::OnMouseDown(const PositionF& pos) 
		{
			Clicked(this, m_label->Get());
		}

		// constructor
		TextItem::TextItem(const std::string& text, UIResources::FontType type)
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

		void TextItem::SetFontType(UIResources::FontType type)
		{
			m_label->SetFontType(type);
		}

		UIResources::FontType TextItem::GetFontType() const
		{
			return m_label->GetFontType();
		}

		// set the text
		void TextItem::Set(const std::string& text)
		{
			m_label->Set(text);
		}

		std::string TextItem::Get() const
		{
			return m_label->Get();
		}

		void TextItem::Draw(const UIDrawContext& context) const 
		{
			context.skin.DrawTextItem(*this, context);
		}

		void TextItem::SetSelected(bool selected)
		{
			m_selected = selected;
		}

		void TextItem::Select()
		{
			m_selected = true;
		}

		void TextItem::Unselect()
		{
			m_selected = false;
		}

		bool TextItem::IsSelected() const
		{
			return m_selected;
		}
#pragma endregion

#pragma region // VectorTextListModel
		// returns the number of text in the list
		size_t VectorTextListModel::Size() const 
		{
			return m_items.size();
		}

		// returns the text at the given index
		const std::string& VectorTextListModel::Get(size_t index) const
		{
			return m_items[index];
		}

		// removes the text at the given index and triggers the Changed event
		void VectorTextListModel::RemoveAt(size_t index)
		{
			assert(index < m_items.size());

			m_items.erase(m_items.begin() + index);
			Changed();
		}

		// inserts text before the given index and triggers the Changed event
		void VectorTextListModel::Insert(size_t index, const std::string& text)
		{
			index = std::min<size_t>(index, m_items.size());

			m_items.insert(m_items.begin() + index, text);
			Changed();
		}

		// appends multiple texts to the end of the list and triggers the Changed event
		void VectorTextListModel::Append(const std::vector<std::string>& texts)
		{
			if (texts.empty())
			{
				return;
			}

			m_items.insert(m_items.end(), texts.begin(), texts.end());
			Changed();
		}

		// appends text to the end of the list and triggers the Changed event
		void VectorTextListModel::Append(const std::string& text) 
		{
			m_items.push_back(text);
			Changed();
		}

		// replaces the text at the given index and triggers the Changed event
		void VectorTextListModel::Set(size_t index, const std::string& text) 
		{
			assert(index < m_items.size());

			m_items[index] = text;
			Changed();
		}

		// clears the list of text and triggers the Changed event
		void VectorTextListModel::Clear() 
		{
			if (m_items.empty())
			{
				return;
			}

			m_items.clear();
			Changed();
		}

		// remove the last text in the list and triggers the Changed event
		void VectorTextListModel::RemoveLast() 
		{
			if (m_items.empty())
			{
				return;
			}

			m_items.pop_back();
			Changed();
		}

		// remove the first text in the list and triggers the Changed event
		void VectorTextListModel::RemoveFirst() 
		{
			if (m_items.empty())
			{
				return;
			}

			m_items.erase(m_items.begin());
			Changed();
		}
#pragma endregion

#pragma region // TextList
		// callback to update the layout of the realized items when the size of the TextList changes.
		void TextList::OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) 
		{
			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		// callback to handle event when a TextItem is clicked
		void TextList::OnClickTextItem(TextItem* item, const std::string& text)
		{
			// do a linear search to find which item is the clicked one. this is fine.
			// realized items list will never be too big that doing this will cause performance issue
			auto it = std::find(
				m_realizedItems.begin(),
				m_realizedItems.end(),
				item);

			// if in case the given TextItem does not exist in our realizedItem list, bail out
			if (it == m_realizedItems.end())
				return;

			// determine the index in the realized item list that is clicked
			int realizedIndex = static_cast<int>(std::distance(m_realizedItems.begin(), it));

			// translate that index into actual text list index
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

					//if the index is found in the list...
					if (it != m_selectedIndices.end())
					{
						// remove it from the index list
						m_selectedIndices.erase(it);

						// if TextList is in "all selected" mode, whatever index is in the list, means it is unselected. 
						// so if it's in the list and we are removing it, it means we are selecting it.
						// but if it's in "not selected" mode, behavior is opposite. if it's in the list and we're removing it,
						// it means its selected and we're unselecting it.
						m_allSelected ? m_realizedItems[realizedIndex]->Select() : m_realizedItems[realizedIndex]->Unselect();
					}
					// if index is not in list...
					else
					{
						// add the index int the list
						m_selectedIndices.insert(modelIndex);

						// if TextList is in "all selected" mode, if index is not in the list, means it is selected. 
						// so if we are adding it to our list, it means we are unselecting it.
						// but if it's in "not selected" mode, behavior is opposite. if it's not in the list and we're adding it,
						// it means its unselected and we're selecting it.
						m_allSelected ? m_realizedItems[realizedIndex]->Unselect() : m_realizedItems[realizedIndex]->Select();
					}
					break;
				}
				default:
					break;
				}
			}

			// raise clicked event
			Clicked(modelIndex, m_realizedItems[realizedIndex]->IsSelected(), text);
		}

		// synchronizes the size of realized items with the number of visible items based on the current min/max range.
		// this does not refresh the text or position of the realized items, it only ensures that the number of realized items matches the number of visible items.
		void TextList::UpdateRealizedItems()
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
				// remove the last realized item until our list have same number of items as our visible range
				m_realizedItems.back()->Clicked -= engine::event::Handler(this, &TextList::OnClickTextItem);
				RemoveChild(m_realizedItems.back());
				m_realizedItems.pop_back();
			}

			// or is our visible labels not enough?
			while (m_realizedItems.size() < numVisible)
			{
				// create new realized item and add to our realized items list. listen to its click event so we update its select state when it's clicked
				std::unique_ptr<TextItem> textItem = std::make_unique<TextItem>("", m_fontType);
				textItem->SetName("TextList.TextItem");
				m_realizedItems.push_back(textItem.get());
				textItem->Clicked += engine::event::Handler(this, &TextList::OnClickTextItem);
				AddChild(std::move(textItem));
			}
		}

		// updates the layout of the realized items, including their position, size, and text
		void TextList::UpdateLayout()
		{
			// if we are currently updating the layout, we don't want to trigger another update. 
			// instead, we mark the layout as dirty and return. this will ensure that the layout is updated after the current update is finished.
			if (m_updating > 0 || !m_dirty) return;

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
		void TextList::OnChangeTextListModel()
		{
			m_dirty = true;
			if (m_updating == 0) UpdateLayout();

			// bubble up the event to notify that the text list has changed. this allows any observers (like a UI component) to react to changes in the data.
			Changed();
		}

		TextList::TextList(float itemHeight, UIResources::FontType fontType)
			: m_itemHeight(itemHeight)
			, m_min(0)
			, m_max(-1)
			, m_textListModel(nullptr)
			, m_fontType(fontType)
			, m_allSelected(false)
			, m_updating(0)
			, m_dirty(false)
		{
			// it's ok for this widget to be movable. on its own it cannot be dragged because it resizes to fit its content which are the realized items. 
			// realized items are TextItem widgets and they are not movable
			m_moveBehavior = MoveBehavior::Free;

			// by default, we will use the internal text list model. this allows the TextList to manage its own data if needed.
			m_textListModel = &m_internalTextListModel;
			m_textListModel->Changed += engine::event::Handler(this, &TextList::OnChangeTextListModel);
		}

		TextList::~TextList()
		{
			// unsubscribe from the text list model's OnChanged event to avoid dangling references and potential crashes when the TextList is destroyed.
			if (m_textListModel)
			{
				m_textListModel->Changed -= engine::event::Handler(this, &TextList::OnChangeTextListModel);
			}
		}

		// bind this TextList to a new text list model. if no model is provided, it will use the internal model.
		void TextList::Bind(ITextListModel* model)
		{
			m_textListModel->Changed -= engine::event::Handler(this, &TextList::OnChangeTextListModel);

			m_textListModel = model ? model : &m_internalTextListModel;

			m_textListModel->Changed += engine::event::Handler(this, &TextList::OnChangeTextListModel);

			// unselect all to be safe since we're changing text data source
			UnselectAll();

			// when we bind to a new model, we need to update the layout to reflect the changes in the underlying data.

			// if we're not deferring layout update, do it now. otherwise, layout update will happen only when called for
			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		bool TextList::IsBoundTo(ITextListModel* model) const
		{
			return m_textListModel == model;
		}

		// remove all items and clear the list
		void TextList::Clear()
		{
			// set min/max to default values. this will ensure that no items are visible and the realized items will be cleared.
			m_min = 0;
			m_max = -1;

			// remove all items from the list and clear the realized items as well as TextList is subscribed to the OnChanged event of the model 
			// which will call UpdateLayout() to clear the realized items and reset the size of the TextList to accommodate the empty list.
			m_textListModel->Clear();

			// clear selection list since we will contain no more items
			UnselectAll();
		}

		// unselect all selected items in list if there's any
		void TextList::UnselectAll()
		{
			// set to "not selected' mode
			m_allSelected = false;

			// set selected state of all TextItems to unselected
			for (TextItem* item : m_realizedItems)
			{
				item->Unselect();
			}

			// since we're now in "not selected" mode and we're unselecting everything, let's clear our selected indices list
			m_selectedIndices.clear();
		}

		// select all items in list if there's any
		void TextList::SelectAll()
		{
			// we do this only if we're in multiple selection mode
			if (m_selectionMode != SelectionMode::Multiple) return;

			// set to "all selected" mode
			m_allSelected = true;

			// since we are in "all selected mode", we are selecting everything. so clearing our selected indices list means all items are selected
			m_selectedIndices.clear();

			// set selected state of all TextItems to selected
			for (TextItem* item : m_realizedItems)
			{
				item->Select();
			}
		}

		// set font type
		void TextList::SetFontType(UIResources::FontType type)
		{
			if (m_fontType == type) return;

			m_fontType = type;

			// of course we need to set font type of each realized items we have
			for (TextItem* item : m_realizedItems)
			{
				item->SetFontType(type);
			}
		}

		// getter for font type
		UIResources::FontType TextList::GetFontType() const
		{
			return m_fontType;
		}

		// set item height
		void TextList::SetItemHeight(float itemHeight)
		{
			if (m_itemHeight == itemHeight) return;
			m_itemHeight = itemHeight;

			// if we're not deferring layout update, do it now. otherwise, layout update will happen only when called for
			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		// getter for item height
		float TextList::GetItemHeight() const
		{
			return m_itemHeight;
		}

		// in cases where layout update needs to be deferred, call this first. any calls that requires layout update will be scoped between BeginUpdate() and EndUpdate()
		void TextList::BeginUpdate()
		{
			m_updating++;
		}

		// closing scope of deferred layout update. call this once all calls requiring layout update are executed and we're ready to update the layout
		void TextList::EndUpdate()
		{
			m_updating--;
			if (m_updating < 0) m_updating = 0;

			// for nested Begin/End update, we only execute layout update when last scope closes
			if (m_updating == 0) UpdateLayout();
		}

		// min can be > max. it just means nothing is visible
		void TextList::SetMin(int min)
		{
			if (m_min == min) return;

			m_min = min;

			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		// max can be < min. it just means nothing is visible
		void TextList::SetMax(int max)
		{
			if (m_max == max) return;

			m_max = max;

			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		// max can be < min. it just means nothing is visible. min can be > max. it just means nothing is visible
		void TextList::SetRange(int min, int max)
		{
			if (m_min == min && m_max == max) return;

			m_min = min;
			m_max = max;

			m_dirty = true;
			if (m_updating == 0) UpdateLayout();
		}

		// getter for min
		int TextList::GetMin() const
		{
			return m_min;
		}

		// getter for max
		int TextList::GetMax() const
		{
			return m_max;
		}

		// add text at the bottom of the list
		void TextList::Append(const std::string& text)
		{
			// adding item to text model will trigger change event which we are subscribed to. our handler will trigger layout update to reflect changes in underlying data
			m_textListModel->Append(text);
		}

		void TextList::RemoveLast()
		{
			m_textListModel->RemoveLast();
		}

		void TextList::RemoveFirst()
		{
			m_textListModel->RemoveFirst();
		}

		void TextList::RemoveAt(size_t index)
		{
			m_textListModel->RemoveAt(index);
		}

		void TextList::Insert(size_t index, const std::string& text)
		{
			m_textListModel->Insert(index, text);
		}

		void TextList::Set(size_t index, const std::string& text)
		{
			m_textListModel->Set(index, text);
		}

		// set selection mode. everytime selection mode changes, all selected items are unselected for safety purposes
		void TextList::SetSelectionMode(SelectionMode mode)
		{
			if (m_selectionMode == mode) return;

			m_selectionMode = mode;

			// everytime selection mode changes, just safely clear currently selected items
			UnselectAll();
		}

		SelectionMode TextList::GetSelectionMode() const
		{
			return m_selectionMode;
		}

		size_t TextList::Size() const
		{
			return m_textListModel->Size();
		}

		void TextList::Draw(const UIDrawContext& context) const 
		{
			context.skin.DrawTextList(*this, context);
		}
#pragma endregion

#pragma region // TextListBox
		// updates the layout of the text list based on the current size of the scroll view's viewport.
		// this will adjust the text list's width to match the scroll view's viewport width and 
		// recalculate the min/max visible items based on the new viewport size, item height, and current viewport offset.
		void TextListBox::UpdateTextListLayout()
		{
			// always adjust the textlist's width to match the scrollview's viewport width. height will be determined by content's own size
			m_textList->SetSize({ m_scrollView->GetViewPortSize().width, m_textList->GetSize().height });

			// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
			UpdateTextListMinMax();
		}

		// recalculate the textlist's min/max visible items based on the new viewport size, item height, and current viewport offset
		void TextListBox::UpdateTextListMinMax()
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

		void TextListBox::OnVerbose(bool enable)
		{
			m_scrollView->Verbose(enable);
			m_textList->Verbose(enable);
		}

		TextListBox::TextListBox(float itemHeight, UIResources::FontType fontType)
			: m_scrollView(nullptr)
			, m_textList(nullptr)
		{
			// create a scroll view and add it as a child. make it always fit into this widget
			std::unique_ptr<ScrollView> scrollview = std::make_unique<ScrollView>();
			scrollview->SetLayoutMode(engine::gui::LayoutMode::Anchor);
			scrollview->SetHorizontalAnchor(engine::gui::HorizontalAnchor::Fill);
			scrollview->SetVerticalAnchor(engine::gui::VerticalAnchor::Fill);
			scrollview->SetMargin({ 0,0,0,0 });
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
			m_textList->Clicked += [&](int index, bool selected, const std::string& text)
				{
					// if selected item is partially visible, reposition so that it is fully visible
					{
						// explanation on how a clicked item that is partially visible becomes fully visible
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

						// get all necessary parameters we will use
						VecF offset = m_scrollView->GetViewPortOffset();
						float itemHeight = m_textList->GetItemHeight();
						SizeF viewportSize = m_scrollView->GetViewPortSize();
						float selPos = index * itemHeight;

						// if clicked item is partially visible and at the top side of the realized items, scroll content to make it fully visible
						if (selPos < offset.y)
						{
							offset.y = selPos;
							m_scrollView->SetOffset(offset);
						}

						// if clicked item is partially visible and at the bottom side of the realized items, scroll content to make it fully visible
						if (selPos + itemHeight > offset.y + viewportSize.height)
						{
							offset.y += (selPos + itemHeight) - (offset.y + viewportSize.height);

							m_scrollView->SetOffset(offset);
						}
					}

					// raise click event
					Clicked(index, selected, text);
				};
		}

		// raise this event when Text Item is clicked and its selected state is toggled
		engine::event::Event<int, bool, const std::string&> Clicked;

		void TextListBox::Append(const std::string& text)
		{
			m_textList->Append(text);
		}

		void TextListBox::RemoveLast()
		{
			m_textList->RemoveLast();
		}

		void TextListBox::SetItemHeight(float itemHeight)
		{
			m_textList->SetItemHeight(itemHeight);
			UpdateTextListLayout();
		}

		float TextListBox::GetItemHeight() const
		{
			return m_textList->GetItemHeight();
		}

		void TextListBox::Draw(const UIDrawContext& context) const 
		{
			context.skin.DrawTextListBox(*this, context);
		}

		void TextListBox::Clear()
		{
			m_textList->Clear();
		}

		void TextListBox::UnselectAll()
		{
			m_textList->UnselectAll();
		}

		void TextListBox::SelectAll()
		{
			m_textList->SelectAll();
		}

		void TextListBox::SetSelectionMode(SelectionMode mode)
		{
			m_textList->SetSelectionMode(mode);
		}

		SelectionMode TextListBox::GetSelectionMode() const
		{
			return m_textList->GetSelectionMode();
		}

		void TextListBox::SetFontType(UIResources::FontType type)
		{
			m_textList->SetFontType(type);
		}

		UIResources::FontType TextListBox::GetFontType() const
		{
			return m_textList->GetFontType();
		}


		// bind this TextList to a new text list model. if no model is provided, it will use the internal model.
		void TextListBox::Bind(ITextListModel* model)
		{
			m_textList->Bind(model);
		}

		bool TextListBox::IsBoundTo(ITextListModel* model) const
		{
			return m_textList->IsBoundTo(model);
		}

#pragma endregion
	}
}