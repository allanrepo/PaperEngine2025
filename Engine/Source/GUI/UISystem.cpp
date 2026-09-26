#include <GUI/UISystem.h>
#include <Utilities/Logger.h>
#include <GUI/UIRenderer.h>


#pragma region // UISystem
namespace engine
{
	namespace gui
	{
#pragma region // UISystem::InteractionStateTracker
		// event handler for widgets that are unregistered from their UI tree. if they are any of the tracked states, they are removed from tracked state
		void UISystem::InteractionStateTracker::OnWidgetUnregistered(Widget* widget)
		{
			if (m_mouseCapture == widget) SetCapture(nullptr);
			if (m_focus == widget) SetFocus(nullptr);
			if (m_mouseOver == widget) SetMouseOver(nullptr);
		}

		// internal helper method to untrack a widget. this can be focus, capture, or hover widget
		void UISystem::InteractionStateTracker::Untrack(Widget* widget)
		{
			if (widget && m_trackedWidgets.Has(widget))
			{
				m_trackedWidgets[widget]--;
				if (m_trackedWidgets[widget] == 0)
				{
					widget->UnregisteredToSystem -= engine::event::Handler(this, &InteractionStateTracker::OnWidgetUnregistered);
					m_trackedWidgets.Unregister(widget);
					LOG("untracked: " + std::to_string(m_trackedWidgets.Size()));
				}
			}
		}

		// internal helper method to track a widget. this can be focus, capture, or hover widget
		void UISystem::InteractionStateTracker::Track(Widget* widget)
		{
			if (widget)
			{
				if (m_trackedWidgets.Has(widget))
				{
					m_trackedWidgets[widget]++;
				}
				else
				{
					m_trackedWidgets.Register(widget, 1);
					widget->UnregisteredToSystem += engine::event::Handler(this, &InteractionStateTracker::OnWidgetUnregistered);
				}
				LOG("tracked: " + std::to_string(m_trackedWidgets.Size()));
			}
		}

		UISystem::InteractionStateTracker::InteractionStateTracker()
			: m_focus(nullptr)
			, m_mouseCapture(nullptr)
			, m_mouseOver(nullptr)
		{

		}

		UISystem::InteractionStateTracker::~InteractionStateTracker()
		{
			// unsubscribed to any widgets that we're tracking
			// NOTE: 
			// what if tracked widgets are already dangling pointers? well we're note handling this.
			// if a widget which is a unique_ptr ends up as dangling, there is a much bigger problem that needs to be dealt with... 				
			for (auto& [widget, num] : m_trackedWidgets)
			{
				widget->UnregisteredToSystem -= engine::event::Handler(this, &InteractionStateTracker::OnWidgetUnregistered);
			}
		}

		void UISystem::InteractionStateTracker::SetFocus(Widget* widget)
		{
			// if we're setting the same widget that is already in focus, do nothing
			if (m_focus == widget) return;

			// since we're changing focus, notify current focus it's about to lose focus
			if (m_focus)
			{
				// untrack the current focus 
				Untrack(m_focus);

				// fire callback lost focus
				m_focus->OnLostFocus();
				m_focus = nullptr;
			}

			// if new focus widget does not exist, and we're not forcing nullptr, bail out
			if (!widget) return;

			// if this widget is not focusable, and we're not forcing nullptr, bail out
			if (!widget->IsFocusable()) return;

			// this new widget is valid to be new focus, notify it
			m_focus = widget;

			if (m_focus)
			{
				// fire callback on focus
				m_focus->OnGotFocus();

				// track the current focus 
				Track(m_focus);
			}
		}

		void UISystem::InteractionStateTracker::SetCapture(Widget* widget)
		{
			// if widget is already the mouse capture, no need to set again
			if (widget == m_mouseCapture) return;

			// untrack the current mouse capture 
			Untrack(m_mouseCapture);

			// set the capture
			m_mouseCapture = widget;

			// track this new mouse capture widget
			Track(m_mouseCapture);
		}

		void UISystem::InteractionStateTracker::SetMouseOver(Widget* widget)
		{
			// if widget is already the mouse over, no need to set again
			if (widget == m_mouseOver) return;

			// untrack the current mouse hover 
			Untrack(m_mouseOver);

			// set mouse over widget
			m_mouseOver = widget;

			// track the current mouse capture 
			Track(m_mouseOver);
		}

		Widget* UISystem::InteractionStateTracker::GetMouseCapture() const
		{
			return m_mouseCapture;
		}

		Widget* UISystem::InteractionStateTracker::GetFocus() const
		{
			return m_focus;
		}

		Widget* UISystem::InteractionStateTracker::GetMouseOver() const
		{
			return m_mouseOver;
		}
#pragma endregion

#pragma region // general operations
		// constructor
		UISystem::UISystem() :
			m_layerManager(this),
			m_dragDropLayer(this)
		{
			// define build for root layer and queue on layer manager
			Layer::BuildDescription root
			{
				PositionF{0,0},
				SizeF{0, 0},
				nullptr,
				Layer::Modal,
				false
			};
			m_layerManager.QueueAdd(root);

			// build the root layer
			m_layerManager.ProcessCommandRequests();
		}

		// destructor
		UISystem::~UISystem()
		{
		}

		// transform and extent setter for root layer
		void UISystem::SetSize(const SizeF& size)
		{
			Root().SetSize(size);
		}

		void UISystem::SetPosition(const PositionF& pos)
		{
			Root().SetPosition(pos);
		}
#pragma endregion

#pragma region // internal UISystem methods
		// root layer getter. only UISystem get to access this.
		Widget& UISystem::Root() const
		{
			return m_layerManager.Bottom();
		}

		// internal layer operation that immediately remove a layer owned by specified widget. it will also collapse layers on top of it
		bool UISystem::RemoveLayer(Widget* owner)
		{
			return m_layerManager.Remove(owner);
		}

		// internal layer operation that toggles a layer owned by specified widget. it is a deferred action
		void UISystem::ToggleLayer(Widget* owner, const Layer::BuildDescription& desc)
		{
			m_layerManager.QueueToggle(owner, desc);
		}

		// internal layer state query
		bool UISystem::IsLayerExpanded(const Widget* owner) const
		{
			return m_layerManager.IsExpanded(owner);
		}

		// internal drag-drop operation where it handles start of a widget being dragged
		void UISystem::BeginDrag(Widget* source)
		{
			// for now we just end drag immediately. we can implement this later when we have drag drop scenario
			// but we want to have this method here as placeholder to show where drag drop manager will be used in
			m_dragDropLayer.Begin(source);
		}

		// the position p here is the mouse position where the widget draggable is dragged into
		// the goal of this method is to identify the top widget that intersects with that position p
		void UISystem::EndDrag(Widget* draggable, const PositionF& p)
		{
			// 1. find the top-most widget that intersects with given point
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// if route result is blocked by modal, it means we intersect outside of existing modal layer and there are no other widgets that can be found to drop current dragged widget
			// but if not modal, we must have found the layer that intersects with  point
			Widget* target = nullptr;
			if (!result.isBlockedByModal)
			{
				// but check first if layer is really valid. it must.
				// if there is no layer found yet we were not blocked by modal, something is wrong. this cannot happen
				if (!result.layer)
				{
					throw std::runtime_error("impossible not to find a layer. why is this so???");
				}

				// let's now find the top widget in this layer's tree that intersects with the point
				target = result.layer->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);
			}


			// 2. pass that widget to dragdrop layer so it will attemp to drop the widget being drag into it
			m_dragDropLayer.End(draggable, target);
		}
#pragma endregion

#pragma region // UISystem input operations
		// external mouse down event handler
		void UISystem::MouseDown(const PositionF& p)
		{
			m_layerManager.FlushCommands();

			// find top layer that intersects with point. layer must be visible and enabled
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// check if result says we're block by modal. this means that a modal layer exist and did not intersect with point and this blocks search to succeeding layer stack
			if (result.isBlockedByModal)
			{
				// if block by modal, collapse above it. we should not collapse modals. it should only be collapsed via command
				m_layerManager.CollapseAbove(result);

				// in case focus, hover and capture are set to widgets that belong to overlay that collapsed, they are reset safely via UnregisterToSystem>Detach
				return;
			}

			// if we reach this point, we should be able to find the top widget that intersects with point. simultaneously we can resolve Z order as we traverse to find the top widget
			// since bottom layer is a modal (root), it should always exist therefore we should always expect a valid layer at this point
			// if not, then we must throw exception as this should not happen
			if (!result.layer)
			{
				throw std::runtime_error("impossible not to find a layer. why is this so???");
			}

			Widget* widget = result.layer->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// at this point, we should have the top-most widget and Z order is resolved. it's impossible to not find top-most widget, we already have the layer.
			if (!widget)
			{
				throw std::runtime_error("why no top-most widget when we already found the layer??");
			}

			// collapse the layer stack above the clicked layer. 
			// it is expected that any layer above the clicked layer will be collapsed. 
			// if a layer trigger e.g. menubutton is clicked, this will collapsed its child layer e.g. submenu if it is open. that is expected behavior
			// if child layer is not open, then nothing will be collapsed. that is expected behavior
			// the next call "mouse down" will handle layer trigger's request to toggle its layer.
			// this command is queued because we don't want to collapse the layer stack above the clicked layer until after the clicked layer's mouse down event is executed. 
			// this is because the clicked layer may request to toggle its layer, and if it does, we don't want to collapse it immediately after mouse down. 
			// we want to give it a chance to toggle its layer first before we collapse the stack above it.
			m_layerManager.QueueCollapseAbove(result);

			// now we are ready to execute MouseDown event on the clicked widget, if there is one. by right there should be one by this time. 
			widget->MouseDown(p);

			// set capture
			m_interactionStateTracker.SetCapture(widget);
			//SetCapture(widget);

			// set focus
			m_interactionStateTracker.SetFocus(widget);
			//SetFocus(widget);

			// hide tooltip. if mouse is down, tooltip should be hidden regardless of where the mouse is clicked
			m_tooltipManager.Hide();
		}

		// external mouse up event handler
		void UISystem::MouseUp(const PositionF& p)
		{
			if (!m_interactionStateTracker.GetMouseCapture()) return;
			m_interactionStateTracker.GetMouseCapture()->MouseUp(p);

			m_interactionStateTracker.SetCapture(nullptr);

			// by right, tooltip of the widget (if it has tooltip) the mouse hovers now should appear... 
			// but after mouse up, we don't have mouse over widget yet, so we don't bother showing tooltip now
		}

		// external mouse move event handler
		void UISystem::MouseMove(const PositionF& p)
		{
			// prioritize captured widget to handle mouse move 
			if (m_interactionStateTracker.GetMouseCapture())
			{
				m_interactionStateTracker.GetMouseCapture()->MouseMove(p);

				// since mouse is captured, tooltip should be hidden
				m_tooltipManager.Hide();

				return;
			}

			// check first if mouse hovers over a overlay
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// if mouse hovers outside of the top overlay in the stack and down to top-most modal overlay, the route result will be "blocked by modal"
			// this is because when one or more modal overlay exists, the top-most modal overlay and succeeding overlays on top of it are the only ones 
			// allowed to receive mouse event or user input in general. if mouse cursor did not hover over any of them overlays, then mouse move is ignored. 
			if (result.isBlockedByModal)
			{
				// just in case there is a mouse over widget somewhere, let's handle its mouse leave
				if (m_interactionStateTracker.GetMouseOver())
				{
					m_interactionStateTracker.GetMouseOver()->MouseLeave();
					m_interactionStateTracker.SetMouseOver(nullptr);
					//m_mouseOver->MouseLeave();
					//SetMouseOver(nullptr);
				}

				// make sure to hide any active tooltip as well
				m_tooltipManager.Hide();

				return;
			}

			// if there is no layer found yet we were not blocked by modal, something is wrong. this cannot happen
			if (!result.layer)
			{
				throw std::runtime_error("impossible not to find a layer. why is this so???");
			}

			// find the top widget in this layer's tree that is hovered. we also include disabled widgets in hover check.
			// reason is so that even disable widgets can still have tooltip shown if they have it
			Widget* hover = result.layer->FindTopWidgetAt(p, Widget::SearchFlags::Visible);

			// let's resolve which widget is mouse over now, if any
			if (hover != m_interactionStateTracker.GetMouseOver())
			{
				// invoke mouse leave on current mouse hover widget
				if (m_interactionStateTracker.GetMouseOver())
				{
					m_interactionStateTracker.GetMouseOver()->MouseLeave();
				}

				// just in case we hover outside of root, assuming root is not desktop, hover will be nullptr
				m_interactionStateTracker.SetMouseOver(hover);
				if (m_interactionStateTracker.GetMouseOver())
				{
					m_interactionStateTracker.GetMouseOver()->MouseEnter();
				}
			}

			// finally if there is a mouse over widget, let it handle mouse move event
			if (m_interactionStateTracker.GetMouseOver())
			{
				m_interactionStateTracker.GetMouseOver()->MouseMove(p);
			}

			// if you reach this point, then mouse hovers a widget that might have a tooltip. show it.
			m_tooltipManager.Show(m_interactionStateTracker.GetMouseOver());
		}

		// keyboard keydown handler
		void UISystem::KeyDown(int key)
		{
			if (m_interactionStateTracker.GetFocus())
			{
				m_interactionStateTracker.GetFocus()->KeyDown(key);
			}
		}

		// keyboard keyup handler
		void UISystem::KeyUp(int key)
		{
			if (m_interactionStateTracker.GetFocus())
			{
				m_interactionStateTracker.GetFocus()->KeyUp(key);
			}
		}
#pragma endregion

#pragma region // layer operations API
		// collapse all active layers and leave only root layer up
		// this is a deferred action. layers will only collapsed when it is ready to collapse it
		void UISystem::Collapse()
		{
			m_layerManager.QueueCollapse(1);
		}

		// spawn a layer on top of the layer tree. good for spawning modal message box
		// this is a deferred action. layer will only be added into layer stack when it is ready to add it
		void UISystem::AddLayer(const Layer::BuildDescription& desc)
		{
			m_layerManager.QueueAdd(desc);
		}
#pragma endregion

#pragma region // scope operations API
		// call this in application update loop along with its End() to close the UISystem's scope
		void UISystem::Begin()
		{
			m_layerManager.FlushCommands();
		}

		void UISystem::End()
		{
			// if a overlay trigger is clicked, it might have requested to toggle its overlay. process those requests here
			m_layerManager.ProcessCommandRequests();
		}
#pragma endregion

#pragma region // Interaction state queries
		bool UISystem::HasFocus(const Widget& widget) const
		{
			return &widget == m_interactionStateTracker.GetFocus();
		}
		bool UISystem::HasMouseCapture(const Widget& widget) const
		{
			return &widget == m_interactionStateTracker.GetMouseCapture();
		}
		bool UISystem::IsMouseOver(const Widget& widget) const
		{
			return &widget == m_interactionStateTracker.GetMouseOver();
		}
#pragma endregion

#pragma region // resource management
		// set font for given resource type
		void UISystem::SetFont(Font font, UIResources::FontType type)
		{
			bool fontChanged = false;
			switch (type)
			{
			case UIResources::FontType::Default:
				if (m_resources.defaultFont != font) fontChanged = true;
				m_resources.defaultFont = font;
				break;
			case UIResources::FontType::Highlight:
				if (m_resources.highlightFont != font) fontChanged = true;
				m_resources.highlightFont = font;
				break;
			case UIResources::FontType::Title:
				if (m_resources.titleFont != font) fontChanged = true;
				m_resources.titleFont = font;
				break;
			default:
				break;
			}

			// update all widgets if font changed as they may need to recalculate their layout based on new font
			if (fontChanged)
			{
				m_layerManager.ForEach([](Widget* widget)
					{
						widget->ForEachWidget([](Widget* widget)
							{
								widget->ResourceChange();
								return true;
							});
					});
			}
		}

		// get font from given resource type
		Font UISystem::GetFont(UIResources::FontType type) const
		{
			switch (type)
			{
			case UIResources::FontType::Default:
				return m_resources.defaultFont;
			case UIResources::FontType::Highlight:
				return m_resources.highlightFont;
			case UIResources::FontType::Title:
				return m_resources.titleFont;
			default:
				throw std::runtime_error("invalid font type");
			}
		}
#pragma endregion

#pragma region // display operations API
		// this makes root layer visible
		void UISystem::Show()
		{
			Root().Show();
		}

		// draw via UIDrawContext
		void UISystem::Draw(UIDrawContext& context)
		{
			// draw layers
			m_layerManager.ForEach([&](Widget* widget)
				{
					UIRenderer::Draw(context, *widget);
				});

			// draw tooltip
			UIRenderer::Draw(context, *m_tooltipManager.Get());

			// draw draggable
			m_dragDropLayer.ForEachChild([&](Widget* widget)
				{
					UIRenderer::Draw(context, *widget);
				});
		}
#pragma endregion

#pragma region // widget operations API
		void UISystem::AddWidget(std::unique_ptr<Widget> widget)
		{
			Root().AddChild(std::move(widget));
		}

		void UISystem::RemoveWidget(Widget* widget)
		{
			// bail out if invalid
			if (!widget) return;

			// we can now remove this widget. this will remove the widget's whole tree. 
			//if (!m_layoutTree.Remove(widget))
			if (!Root().Remove(widget))
			{
				// let's be strict for now to catch any silent error
				throw std::runtime_error("failed to remove a widget from root");
			}
		}
#pragma endregion

	}
}
#pragma endregion