#include <GUI/gui.h>

namespace engine
{
	namespace gui
	{
#pragma region // Layer
		// overriden from Widget to return the UISystem as Layer is a root widget and has no parent.  
		UISystem* Layer::GetSystem() const
		{
			return m_system;
		}

		Layer::Layer(UISystem* system, Widget* owner, const PositionF& pos, const SizeF& size, const Type& type, bool movable) :
			m_owner(owner),
			m_system(system),
			m_type(type)
		{
			// can be movable or not. 
			m_moveBehavior = movable ? Widget::MoveBehavior::Free : Widget::MoveBehavior::None;

			// set the position and size of the layer. this is usually done by the builder function, but can be set here as well.
			SetPosition(pos);
			SetSize(size);

			// layer is not focusable. it is a root widget and responsible for the extent of its children
			m_focusable = false;
		}

		Widget* Layer::GetOwner() const
		{
			return m_owner;
		}

		bool Layer::IsModal() const
		{
			return m_type == Type::Modal;
		}

		bool Layer::IsMenu() const
		{
			return m_type == Type::Menu;
		}

		bool Layer::IsPopup() const
		{
			return m_type == Type::Popup;
		}

		Layer::Type Layer::GetType() const
		{
			return m_type;
		}

		void Layer::Draw(const UIDrawContext& context) const
		{
			if (context.skin) context.skin->DrawLayer(*this, context);
		}
#pragma endregion

#pragma region // LayerStack
		// constructor is protected to enforce that only LayerManager can create and manipulate the LayerStack.
		LayerStack::LayerStack()
		{
		}

		// returns the number of layers in the stack. this is used to determine if there are any active layers in the stack.
		size_t LayerStack::Size() const
		{
			return m_layers.size();
		}

		// Collapses the layer stack starting at the specified layer index.
		//
		// ---------------------------------------------------------------------------------
		// DESIGN NOTES
		// ---------------------------------------------------------------------------------
		// Layer collapse is always performed from a layer downward toward the top
		// of the layer stack.
		//
		// Example:
		//
		//	Stack:
		//		[Layer A]
		//		[Layer B]
		//		[Layer C]
		//
		//	CollapseAt(B)
		//
		//	Result:
		//		[Layer A]
		//
		// Layer B and all layers above it are removed.
		//
		// ---------------------------------------------------------------------------------
		// CASCADED LAYER COLLAPSE
		// ---------------------------------------------------------------------------------
		//
		// Layers may contain Layer owners that own child layers higher in the stack.
		//
		// Example:
		//
		//	Layer A
		//		contains Trigger B
		//
		//	Layer B
		//		contains Trigger C
		//
		//	Layer C
		//
		// During collapse, layer's UI tree widgets are first unregistered from the UISystem
		// before the layer itself is erased from the layer stack.
		//
		// While unregistering:
		//
		//	OwnerWidget::OnUnregisterToSystem()
		//		-> UISystem::UnregisterLayer()
		//			-> CollapseByOwner()
		//
		// may recursively request collapse of child layers higher in the stack.
		//
		// This is safe because:
		//	- layer ownership is acyclic
		//	- layer stack destruction only proceeds upward
		//	- layer mutations only remove suffixes of the layer stack
		//	- traversal is index-based (not iterator-based)
		//	- the layer stack is never reordered during collapse
		//
		// Example:
		//
		//	Initial stack:
		//		[A][B][C]
		//
		//	CollapseAt(A)
		//
		//	1. A unregisters Trigger B
		//	2. Trigger B collapses B
		//	3. B unregisters Trigger C
		//	4. Trigger C collapses C
		//
		// Each nested collapse only removes layers above the current layer.
		//
		// Because nested collapses only shrink the end of the layer stack,
		// the outer forward traversal remains valid and will naturally terminate
		// once the layer stack size becomes smaller than the current traversal index.
		//
		// ---------------------------------------------------------------------------------
		// IMPORTANT INVARIANT
		// ---------------------------------------------------------------------------------
		//
		// This method is only safe because layer collapse semantics are strictly:
		//
		//	- synchronous
		//	- upward-only
		//	- suffix-removing
		//
		// Future changes such as below may invalidate these assumptions and require a deferred mutation model.
		//	- arbitrary overlay removal
		//	- overlay insertion during collapse
		//	- overlay reordering
		//	- deferred destruction
		//	- async/evented mutation
		void LayerStack::CollapseAt(const Route& result)
		{
			int index = result.index < 0 ? 0 : result.index;

			// index can be out of bounds. if there are no active layers, and this is called, if index = 0, then this condition is valid
			if (index >= (int)m_layers.size()) return;

			// since we're removing layers, their children must unregister to system.
			for (size_t i = index; i < m_layers.size(); i++)
			{
				m_layers[i]->RemoveChildren();
			}

			// after unregistering layers' tree, remove them 
			m_layers.erase(m_layers.begin() + index, m_layers.end());
		}

		// a variant of CollapseAt() that collapses the layer stack starting at the layer above the specified layer index.
		void LayerStack::CollapseAbove(const Route& route)
		{
			Route routeAbove = route;
			routeAbove.index++;
			CollapseAt(routeAbove);
		}

		// collapses the entire layer stack. this is equivalent to CollapseAt(0)
		void LayerStack::Collapse()
		{
			Route route;
			route.index = 0;
			CollapseAt(route);
		}

		// this is the only way to add a new overlay in the stack and it will always end it at the end of the stack
		void LayerStack::Add(std::unique_ptr<Layer> overlay)
		{
			m_layers.push_back(std::move(overlay));
		}

		// finds the top-most active layer that is owned by the given owner widget. 
		// if found, returns a Route with the layer and index. if not found, returns a Route with nullptr and -1
		LayerStack::Route LayerStack::FindRouteByOwner(Widget* owner)
		{
			Route result;

			// check if any active layer is owned by given owner
			for (int i = 0; i < m_layers.size(); i++)
			{
				// if this widget is an owner of existing layer, then layer is active. 
				if (m_layers[i].get()->GetOwner() == owner)
				{
					result.layer = m_layers[i].get();
					result.target = m_layers[i].get();
					result.index = i;
					break;
				}
			}

			return result;
		}

		// collapses layer stack on layer with the specified owner widget
		void LayerStack::CollapseByOwner(Widget* owner)
		{
			// find the active layer that is owned by given owner, if any
			Route result = FindRouteByOwner(owner);
			if (!result.layer) return;

			// if found, since you get the index, create Route and set the index. collapse on it
			CollapseAt(result);
		}

		// get the bottom-most layer in the stack. if the stack is empty, throws an exception
		Layer& LayerStack::Bottom() const
		{
			if (m_layers.empty())
			{
				throw std::runtime_error("Querying an empty stack is wrong.");
			}

			return *m_layers.front().get();
		}

		// get the top-most layer in the stack. if the stack is empty, throws an exception
		Layer& LayerStack::Top() const
		{
			if (m_layers.empty())
			{
				throw std::runtime_error("Querying an empty stack is wrong.");
			}

			return *m_layers.back().get();
		}

		// find which top-most active layer that intersects with given point
		LayerStack::Route LayerStack::FindRouteFromTopAt(const PositionF& position, int flags)
		{
			Route result;

			// since we are looking for the top-most layer, we need to traverse the stack from top to bottom
			for (int i = (int)m_layers.size() - 1; i >= 0; i--)
			{
				// find the top-most widget in this layer that intersects with given point.
				Widget* widget = m_layers[i]->FindTopWidgetAt(position, flags);

				// if found, return it with the layer and index
				if (widget)
				{
					result.target = widget;
					result.index = i;
					result.layer = m_layers[i].get();
					result.isBlockedByModal = false;
					break;
				}
				// if this layer did not intersect with point, check if it's modal
				else
				{
					// is this layer a modal? if yes, stop right here. modal layers when active is the only widget that can absorb user input
					if (m_layers[i]->IsModal())
					{
						result.index = i;
						result.isBlockedByModal = true;
						break;
					}
				}
			}

			return result;
		}

		// check if any active layer is owned by given owner widget. returns true if found, false otherwise
		bool LayerStack::IsExpanded(const Widget* owner) const
		{
			// check if any active layer is owned by given owner
			for (int i = 0; i < m_layers.size(); i++)
			{
				// if this widget is an owner of existing layer, then layer is active. 
				if (m_layers[i].get()->GetOwner() == owner)
				{
					return true;
				}
			}

			return false;
		}
#pragma endregion

#pragma region // LayerManager
		// constructor. it requires a UISystem pointer to associate this manager with the UI system. 
		// this is used to access the UISystem for registering and unregistering layers' UI trees.
		LayerManager::LayerManager(UISystem* system) :
			m_system(system)
		{
		}

		// collapses the layer stack starting at the specified layer index. this is a wrapper around LayerStack::CollapseAbove()
		void LayerManager::CollapseAbove(const LayerStack::Route& route)
		{
			m_stack.CollapseAbove(route);
		}

		// finds the top-most layer that intersects with given position and valid with given flags
		LayerStack::Route LayerManager::FindRouteFromTopAt(const PositionF& position, int flags)
		{
			return m_stack.FindRouteFromTopAt(position, flags);
		}

		// collapses the entire layer stack. this is a wrapper around LayerStack::Collapse()
		void LayerManager::Collapse()
		{
			m_stack.Collapse();
		}

		// flushes all pending commands in the command queue
		void LayerManager::FlushCommands()
		{
			m_commands.clear();
		}

		// given a overlay stack route result, let overlay tree handle mouse down by performing overlay stack collapse if needed, 
		// and process on queue overlay command requests e.g. toggle up/down a overlay
		void LayerManager::ProcessCommandRequests()
		{
			// handle overlay add/remove queue requests
			for (Command& cmd : m_commands)
			{
				switch (cmd.command)
				{
					// remove/toggle off the overlay that is owned by widget from overlay request
				case Command::Remove:
				{
					// we already have the index of the overlay stack that we want to collapsed at. just validate and collapse with it
					if (cmd.index >= 0 && cmd.index < m_stack.Size())
					{
						LayerStack::Route route{};
						route.index = cmd.index;
						m_stack.CollapseAt(route);
					}
					break;
				}
				// add this overlay on top of stack
				case Command::Add:
				{
					// create the overlay
					std::unique_ptr<Layer> overlay = std::make_unique<Layer>(m_system, cmd.owner, cmd.position, cmd.size, cmd.type, cmd.movable);

					// if it has a payload, build it and add to overlay as child
					if (cmd.builder)
					{
						cmd.builder(overlay.get());
					}

					// finally, add overlay to top of stack
					m_stack.Add(std::move(overlay));
					break;
				}
				// collapse the entire overlay stack
				case Command::Collapse:
				{
					m_stack.Collapse();
					break;
				}
				default:
					break;
				}
			}

			// flush the commands after consuming them
			m_commands.clear();
		}

		// toggle the overlay
		void LayerManager::QueueToggle(Widget* owner, const Layer::BuildDescription& desc)
		{
			// check if there is an active overlay that is owned by given owner
			LayerStack::Route result = m_stack.FindRouteByOwner(owner);

			// if the owner's overlay is already active, queue it for removal/collapse
			if (result.layer)
			{
				Command cmd{};
				cmd.command = Command::Remove;
				cmd.index = result.index;
				cmd.owner = owner;
				m_commands.push_back(cmd);
				return;
			}

			// create overlay build request
			Command cmd{};
			cmd.command = Command::Add;
			cmd.owner = owner;
			cmd.position = owner->GetAbsolutePosition() + desc.position;
			cmd.size = desc.size;
			cmd.builder = desc.builder;
			cmd.type = desc.type;
			cmd.movable = desc.movable;
			m_commands.push_back(cmd);
		}

		// unregister a overlay build description owned by given widget
		bool LayerManager::Remove(Widget* owner)
		{
			// collapse overlay stack at the overlay of this owner, if any
			m_stack.CollapseByOwner(owner);

			return true;
		}

		// queue add overlay based on build description as this has no owner
		void LayerManager::QueueAdd(const Layer::BuildDescription& desc)
		{
			// create overlay build command on top of stack based on build description
			Command cmd{};
			cmd.command = Command::Add;
			cmd.owner = nullptr;
			cmd.position = desc.position;
			cmd.size = desc.size;
			cmd.builder = desc.builder;
			cmd.type = desc.type;
			cmd.movable = desc.movable;
			m_commands.push_back(cmd);
		}

		// queue collapse overlay stack at given index. if index is not specified, it will collapse the entire stack
		void LayerManager::QueueCollapse(int index)
		{
			Command cmd{};
			cmd.command = Command::Remove;
			cmd.index = index;
			m_commands.push_back(cmd);
		}

		// get the bottom-most layer in the stack. if the stack is empty, throws an exception
		Layer& LayerManager::Bottom() const
		{
			if (!m_stack.Size())
			{
				throw std::runtime_error("stack is empty. querying for first is wrong");
			}

			return m_stack.Bottom();
		}

		// checks if any active layer is owned by given owner widget. returns true if found, false otherwise
		bool LayerManager::IsExpanded(const Widget* owner) const
		{
			return m_stack.IsExpanded(owner);
		}
#pragma endregion
	}
}
