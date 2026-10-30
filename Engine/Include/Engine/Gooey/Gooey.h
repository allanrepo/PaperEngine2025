#pragma once
#include <GUI/Controls.h>

namespace engine
{
	namespace gooey
	{
#pragma region // forward declaration
		class UISystem;
		class Control;
		class Image;
		class Label;
		class Button;	
		class Dialog;
#pragma endregion

#pragma region // Control
		class Control
		{
		protected:
			virtual engine::gui::Widget* GetWidget() const = 0;

		public:
			float GetWidth() const
			{
				return GetWidget()->GetWidth();
			}

			float GetHeight() const
			{
				return GetWidget()->GetHeight();
			}

			SizeF GetSize() const
			{
				return GetWidget()->GetSize();
			}

			void SetSize(const SizeF& size)
			{
				GetWidget()->SetSize(size);
			}

			PositionF GetAbsolutePosition() const
			{
				return GetWidget()->GetAbsolutePosition();
			}

			void SetPosition(const PositionF& pos)
			{
				GetWidget()->SetPosition(pos);
			}

			PositionF GetPosition() const
			{
				return GetWidget()->GetPosition();
			}

			RectF GetAbsoluteRect() const
			{
				return GetWidget()->GetAbsoluteRect();
			}
		};
#pragma endregion

#pragma region // Label
		class Label : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::Label* m_label = nullptr;

			Label(engine::gui::Label* label)
				:m_label(label)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_label;
			}

		public:
			// absolute position of text's top-left corner
			PositionF GetTextAbsolutePosition() const
			{
				return m_label->GetTextAbsolutePosition();
			}

			// set font type
			void SetFontType(UIResources::FontType type)
			{
				return m_label->SetFontType(type);
			}

			// get font type
			UIResources::FontType GetFontType() const
			{
				return m_label->GetFontType();
			}

			// get text
			std::string Get() const
			{
				return m_label->Get();
			}

			// set text
			void Set(const std::string& text)
			{
				m_label->Set(text);
			}

			// set alignment
			void SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign)
			{
				m_label->SetAlignment(vAlign, hAlign);
			}
		};
#pragma endregion

#pragma region // Image
		class Image : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::Image* m_image = nullptr;

			Image(engine::gui::Image* image)
				:m_image(image)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_image;
			}

		public:
			void EnableStretch(bool stretch)
			{
				m_image->EnableStretch(stretch);
			}

			bool IsStretched() const
			{
				return m_image->IsStretched();
			}

			// absolute position of image's top-left corner
			PositionF GetImageAbsolutePosition() const
			{
				return m_image->GetImageAbsolutePosition();
			}

			// get sprite 
			Sprite Get() const
			{
				return m_image->Get();
			}

			// set alignment
			void SetAlignment(VerticalAlignment vAlign, HorizontalAlignment hAlign)
			{
				m_image->SetAlignment(vAlign, hAlign);
			}
		};
#pragma endregion

#pragma region // Button
		class Button : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::Button* m_button = nullptr;

			Button(engine::gui::Button* button)
				:m_button(button)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_button;
			}

		public:
			event::Event<>& Click()
			{
				return m_button->Click;
			}
		};
#pragma endregion

#pragma region // Slider
		class Slider : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::Slider* m_slider = nullptr;

			Slider(engine::gui::Slider* slider)
				:m_slider(slider)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_slider;
			}

		protected:

		public:
			engine::event::Event<float>& Change()
			{
				return m_slider->Change;
			}

			// Sets whether the slider is horizontal or vertical.
			void Horizontal(bool enable)
			{
				m_slider->Horizontal(enable);
			}

			// Returns true if the slider is horizontal, false if it is vertical.
			bool Horizontal() const
			{
				return m_slider->Horizontal();
			}

			// Sets the length of the thumb along the slider's direction.
			void SetThumbLength(float length)
			{
				m_slider->SetThumbLength(length);
			}

			// Sets the minimum value of the slider.
			void Min(float min)
			{
				m_slider->Min(min);
			}

			// Sets the maximum value of the slider.
			void Max(float max)
			{
				m_slider->Max(max);
			}

			// Returns the minimum value of the slider.
			float Min() const
			{
				return m_slider->Min();
			}

			// Returns the maximum value of the slider.
			float Max() const
			{
				return m_slider->Max();
			}

			// Sets the current value of the slider.
			void Value(float value)
			{
				m_slider->Value(value);
			}

			// Returns the current value of the slider.
			float Value() const
			{
				return m_slider->Value();
			}

			// Sets the step size of the slider.
			void SetStepSize(float size)
			{
				m_slider->SetStepSize(size);
			}

			// Sets the number of steps of the slider.
			void SetStepCount(int count)
			{
				m_slider->SetStepCount(count);
			}
		};
#pragma endregion

#pragma region // Scrollbar
		class ScrollBar : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::ScrollBar* m_scrollBar = nullptr;

			ScrollBar(engine::gui::ScrollBar* scrollBar)
				:m_scrollBar(scrollBar)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_scrollBar;
			}

		protected:

		public:
			// event that is fired when the scrollbar is scrolled. it will pass the new offset value to the subscribers. 
			// this is used to notify the content that it needs to update its position based on new offset
			engine::event::Event<float>& Scroll()
			{
				return m_scrollBar->Scroll;
			}

			// constructor
			ScrollBar(float contentLength, float viewportLength, bool isHorizontal);

			// set the offset of the scrollbar. this will update the thumb position and fire the scroll event if the offset is changed.
			void SetOffset(float offset)
			{
				m_scrollBar->SetOffset(offset);
			}

			// getter for offset. this is the current scroll position of the scrollbar. it is used to determine how much content is scrolled.
			float Offset() const
			{
				return m_scrollBar->GetOffset();	
			}

			// set the content length of the scrollbar. this will update the thumb size and position based on new content length and offset.
			void SetContentLength(float length)
			{
				m_scrollBar->SetContentLength(length);
			}

			// set the viewport length of the scrollbar. this will update the thumb size and position based on new viewport length and offset.
			void SetViewportLength(float length)
			{
				m_scrollBar->SetViewportLength(length);
			}

			float GetContentLength() const
			{
				return m_scrollBar->GetContentLength();
			}

			float GetViewportLength() const
			{
				return m_scrollBar->GetViewportLength();
			}
		};
#pragma endregion

#pragma region // ResizeableFrame
		class ResizeableFrame : public Control
		{
		private:
			friend class ControlFactory;

			engine::gui::ResizeableFrame* m_resizeableFrame = nullptr;

			ResizeableFrame(engine::gui::ResizeableFrame* resizeableFrame)
				:m_resizeableFrame(resizeableFrame)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_resizeableFrame;
			}

		protected:

		public:

		};
#pragma endregion

#pragma region // IControlFactory
		class IControlFactory
		{
		protected:
			virtual void AddWidget(std::unique_ptr<engine::gui::Widget> widget) = 0;

		public:
			virtual ~IControlFactory() = default;

			virtual Label MakeLabel(const std::string& string) = 0;
			virtual Image MakeImage(Animated animated) = 0;
			virtual Image MakeImage(Renderable renderable) = 0;
			virtual Button MakeButton() = 0;
			virtual Slider MakeSlider(float min, float max, float thumbLength) = 0;
			virtual ScrollBar MakeScrollBar(float contentLength, float viewportLength, bool isHorizontal) = 0;
			virtual ResizeableFrame MakeResizeableFrame(float borderSize = 20.0f, const SizeF& minSize = { 200, 200 }) = 0;
		};
#pragma endregion

#pragma region // ControlFactory
		class ControlFactory
			: public IControlFactory
		{
		public:
			Label MakeLabel(const std::string& string) override final
			{
				std::unique_ptr<engine::gui::Label> labelWidget = std::make_unique<engine::gui::Label>(string);
				Label label(labelWidget.get());
				AddWidget(std::move(labelWidget));
				return label;
			}

			Image MakeImage(Animated animated) override final
			{
				std::unique_ptr<engine::gui::Image> imageWidget = std::make_unique<engine::gui::Image>(animated);
				Image image(imageWidget.get());
				AddWidget(std::move(imageWidget));
				return image;
			}

			Image MakeImage(Renderable renderable) override final
			{
				std::unique_ptr<engine::gui::Image> imageWidget = std::make_unique<engine::gui::Image>(renderable);
				Image image(imageWidget.get());
				AddWidget(std::move(imageWidget));
				return image;
			}

			Button MakeButton() override final
			{
				std::unique_ptr<engine::gui::Button> buttonWidget = std::make_unique<engine::gui::Button>();
				Button button(buttonWidget.get());
				AddWidget(std::move(buttonWidget));
				return button;
			}

			Slider MakeSlider(float min, float max, float thumbLength) override final
			{
				std::unique_ptr<engine::gui::Slider> sliderWidget = std::make_unique<engine::gui::Slider>(min, max, thumbLength);
				Slider slider(sliderWidget.get());
				AddWidget(std::move(sliderWidget));
				return slider;
			}

			ScrollBar MakeScrollBar(float contentLength, float viewportLength, bool isHorizontal) override final
			{
				std::unique_ptr<engine::gui::ScrollBar> scrollBarWidget = std::make_unique<engine::gui::ScrollBar>(contentLength, viewportLength, isHorizontal);
				ScrollBar scrollBar(scrollBarWidget.get());
				AddWidget(std::move(scrollBarWidget));
				return scrollBar;
			}
			ResizeableFrame MakeResizeableFrame(float borderSize = 20.0f, const SizeF& minSize = { 200, 200 }) override final
			{
				std::unique_ptr<engine::gui::ResizeableFrame> resizeableFrameWidget = std::make_unique<engine::gui::ResizeableFrame>(borderSize, minSize);
				ResizeableFrame resizeableFrame(resizeableFrameWidget.get());
				AddWidget(std::move(resizeableFrameWidget));
				return resizeableFrame;
			}

		};
#pragma endregion

#pragma region // Dialog
		class Dialog
			: public Control
			, public ControlFactory
		{
		private:
			friend class UISystem;

			engine::gui::Dialog* m_dialog = nullptr;

			Dialog(engine::gui::Dialog* dialog)
				:m_dialog(dialog)
			{
			}

			engine::gui::Widget* GetWidget() const override final
			{
				return m_dialog;
			}

		protected:
			void AddWidget(std::unique_ptr<engine::gui::Widget> widget) override final
			{
				m_dialog->AddContent(std::move(widget));
			}
		public:
		};
#pragma endregion

#pragma region // UISystem
		class UISystem
			: public ControlFactory
		{
		private:
			engine::gui::UISystem m_system;

		protected:
			void AddWidget(std::unique_ptr<engine::gui::Widget> widget) override final
			{
				m_system.AddWidget(std::move(widget));
			}

		public:
			UISystem()
			{

			}

			void Draw(const UISkin& skin, IRenderer& renderer)
			{
				engine::gui::UIDrawContext context{ renderer, m_system, skin };
				m_system.Draw(context);
			}

			// external mouse down event handler
			void MouseDown(const PositionF& p)
			{
				m_system.MouseDown(p);
			}

			// external mouse up event handler
			void MouseUp(const PositionF& p)
			{
				m_system.MouseUp(p);
			}

			// external mouse move event handler
			void MouseMove(const PositionF& p)
			{
				m_system.MouseMove(p);
			}

			// keyboard keydown handler
			void KeyDown(int key)
			{
				m_system.KeyDown(key);
			}

			// keyboard keyup handler
			void KeyUp(int key)
			{
				m_system.KeyUp(key);
			}

			void Collapse()
			{
				m_system.Collapse();
			}

			void Begin()
			{
				m_system.Begin();
			}

			void End()
			{
				m_system.End();
			}

			void SetSize(const SizeF& size)
			{
				m_system.SetSize(size);
			}

			void SetPosition(const PositionF& pos)
			{
				m_system.SetPosition(pos);
			}

			void Show()
			{
				m_system.Show();
			}

			// set font for given resource type
			void SetFont(Font font, UIResources::FontType type)
			{
				m_system.SetFont(font, type);
			}

			// get font from given resource type
			Font GetFont(UIResources::FontType type) const
			{
				return m_system.GetFont(type);
			}

			Dialog MakeDialog()
			{
				std::unique_ptr<engine::gui::Dialog> dialog = std::make_unique<engine::gui::Dialog>();
				Dialog dlg(dialog.get());
				AddWidget(std::move(dialog));
				return dlg;
			}
		};
#pragma endregion


	}
}

