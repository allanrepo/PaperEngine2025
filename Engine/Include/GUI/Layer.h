#pragma once
#include <GUI/Widget.h>
#include <Spatial/Position.h>
#include <Math/Size.h>	
#include <Core/Event.h>
#include <Math/Rect.h>

#include <vector>
#include <memory>
#include <functional>

namespace engine
{
	namespace gui
	{
#pragma region // forward declarations
#pragma endregion

#pragma region // Layer
		// UI is not just a single UI tree. it is a stack of UI trees. each tree is a layer. this class represents a layer in the UI stack.
		// Layer is a widget that is the root of a UI tree. Hence, it has no parent. 
		// It can be owned by another widget, which is usually a trigger widget that opens the layer. a Layer without owner is a top-level layer. 
		// It can be of different types: Popup, Modal, Menu, SubMenu. Each type has different behavior in terms of input handling and rendering.
		class Layer : public Widget
		{
		public:
			enum Type
			{
				// Popup layer is a layer that automatically closes when it loses focus. It is used for context menus, tooltips, etc.
				Popup,

				// Modal layer is a layer that blocks input to layers below it. It is used for dialogs, message boxes, etc.
				Modal,

				// Menu layer is a layer that is used for menus. It can be a top-level menu or a submenu. It is used for menu bars, context menus, etc.
				Menu,

				// SubMenu layer is a layer that is used for submenus. It is a child of a Menu layer. It is used for cascading menus, etc.
				SubMenu
			};

		private:
			// only LayerStack can create and manipulate the Layer. hence, LayerStack is a friend class of Layer
			friend class LayerStack;

			// the widget that owns this layer. it can be nullptr if the layer is a top-level layer.
			Widget* m_owner;

			// the UISystem that this layer belongs to. it is used to register and unregister the layer's UI tree.
			UISystem* m_system;

			// the type of the layer. it determines how the layer behaves in terms of input handling and rendering.
			Type m_type;

		protected:
			// overriden from Widget to return the UISystem as Layer is a root widget and has no parent.  
			UISystem* GetSystem() const override final;

		public:
			// recipe for building a layer. this is used to create a layer with specific properties and a builder function that constructs the layer's content.
			struct BuildDescription
			{
				PositionF position = {};
				SizeF size = {};
				std::function<void(Widget*)> builder = nullptr;
				Type type = Type::Popup;
				bool movable = false;
			};

			// constructor
			Layer(UISystem* system, Widget* owner, const PositionF& pos, const SizeF& size, const Type& type, bool movable);

			// getters and type checkers
			Widget* GetOwner() const;
			bool IsModal() const;
			bool IsMenu() const;
			bool IsPopup() const;
			Type GetType() const;

			// draws the layer
			void Draw(const UIDrawContext& context) const override;
		};
#pragma endregion

#pragma region // LayerStack
		// this class contains a stack of layers. it also enforces policies on how layers are collapsed and expanded.
		// only LayerManager can create and manipulate the LayerStack.
		class LayerStack
		{
		protected:
			// only LayerManager can create and manipulate the LayerStack.
			friend class LayerManager;

		public:
			// data structure that represents the result of a search for a layer in the stack. 
			struct Route
			{
				// the layer that was found in the stack. if no layer was found, this will be nullptr
				Widget* layer = nullptr;

				// the widget in the layer's UI tree that was hit by the search. if no widget was hit, this will be nullptr
				Widget* target = nullptr;

				// the index of the layer in the stack. first (bottom) layer has index 0. next is 1. if no layer was found, this will be -1
				int index = -1;

				// indicates if the layer is blocked by a modal layer above it in the stack. if true, the layer cannot receive input events
				bool isBlockedByModal = false;
			};

		private:
			// only LayerManager can create and manipulate the LayerStack.
			friend class LayerManager;

			// the stack of layers. the first layer in the vector is the bottom layer. the last layer in the vector is the top layer.
			std::vector<std::unique_ptr<Layer>> m_layers;

		protected:
			// constructor is protected to enforce that only LayerManager can create and manipulate the LayerStack.
			LayerStack();

			// returns the number of layers in the stack. this is used to determine if there are any active layers in the stack.
			size_t Size() const;

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
			void CollapseAt(const Route& result);

			// a variant of CollapseAt() that collapses the layer stack starting at the layer above the specified layer index.
			void CollapseAbove(const Route& route);

			// collapses the entire layer stack. this is equivalent to CollapseAt(0)
			void Collapse();

			// this is the only way to add a new overlay in the stack and it will always end it at the end of the stack
			void Add(std::unique_ptr<Layer> overlay);

			// finds the top-most active layer that is owned by the given owner widget. 
			// if found, returns a Route with the layer and index. if not found, returns a Route with nullptr and -1
			Route FindRouteByOwner(Widget* owner);

			// collapses layer stack on layer with the specified owner widget
			void CollapseByOwner(Widget* owner);

			// traverse through the layer stack from bottom to top
			template<typename Func>
			void ForEach(const Func& func)
			{
				for (std::vector<std::unique_ptr<Layer>>::iterator it = m_layers.begin(); it != m_layers.end(); it++)
				{
					func(it->get());
				}
			}

			// get the bottom-most layer in the stack. if the stack is empty, throws an exception
			Layer& Bottom() const;

			// get the top-most layer in the stack. if the stack is empty, throws an exception
			Layer& Top() const;

			// find which top-most active layer that intersects with given point
			Route FindRouteFromTopAt(const PositionF& position, int flags);

			// check if any active layer is owned by given owner widget. returns true if found, false otherwise
			bool IsExpanded(const Widget* owner) const;

		};
#pragma endregion

#pragma region // LayerManager
		// this class manages the layer stack and provides an interface for adding, removing, and collapsing layers. 
		// it also handles the queuing of commands to manipulate the layer stack.
		class LayerManager
		{
		private:
			// only UISystem can create and manipulate the LayerManager. this is to ensure that the LayerManager is always associated with a UISystem.
			friend class UISystem;

			// internal data structure to store command request to add, remove, or collapse layers. this is used to queue commands that will be processed later.
			struct Command
			{
				// the type of command to be executed
				enum Type
				{
					Add,
					Remove,
					Collapse,
					CollapseAbove,
				};

				// command type
				Type command;

				// the owner widget that requested the command. this is used to identify which layer to add or remove
				Widget* owner = nullptr;

				// the index of the layer in the stack. this is used to identify which layer to remove or collapse
				int index;

				// the position and size of the layer to be added. this is used to create a new layer with the specified properties
				PositionF position;
				SizeF size;

				// the builder function that will be called to build the content of the layer. this is used to create the UI tree of the layer
				std::function<void(Widget*)> builder;

				// the type of the layer to be added. this is used to determine the behavior of the layer in terms of input handling and rendering
				Layer::Type type;

				// indicates if the layer to be added is movable. this is used to determine if the layer can be dragged around by the user
				bool movable;
			};

			// the stack of layers that this manager manages. this is used to keep track of the active layers in the UI
			LayerStack m_stack;

			// the UISystem that this manager is associated with. 
			UISystem* m_system;

			// the queue of commands that will be processed later. this is used to defer the execution of commands until the appropriate time
			std::vector<Command> m_commands;

		protected:
			// constructor. it requires a UISystem pointer to associate this manager with the UI system. 
			// this is used to access the UISystem for registering and unregistering layers' UI trees.
			LayerManager(UISystem* system);

			// collapses the layer stack starting at the specified layer index. this is a wrapper around LayerStack::CollapseAbove()
			void CollapseAbove(const LayerStack::Route& route);

			// queues a command to collapse the layer stack starting at the specified layer index. this is a wrapper around LayerStack::CollapseAbove()
			void QueueCollapseAbove(const LayerStack::Route& route);

			// finds the top-most layer that intersects with given position and valid with given flags
			LayerStack::Route FindRouteFromTopAt(const PositionF& position, int flags);

			// collapses the entire layer stack. this is a wrapper around LayerStack::Collapse()
			void Collapse();

			// flushes all pending commands in the command queue
			void FlushCommands();

			// handle layer stack collapse and expand command requests.
			void ProcessCommandRequests();

			// toggle the overlay
			void QueueToggle(Widget* owner, const Layer::BuildDescription& desc);

			// unregister a overlay build description owned by given widget
			bool Remove(Widget* owner);

			// traverse through the overlay stack from bottom to top
			template<typename Func>
			void ForEach(const Func& func)
			{
				m_stack.ForEach(func);
			}

			// queue add overlay based on build description as this has no owner
			void QueueAdd(const Layer::BuildDescription& desc);

			// queue collapse overlay stack at given index. if index is not specified, it will collapse the entire stack
			void QueueCollapse(int index = 0);

			// get the bottom-most layer in the stack. if the stack is empty, throws an exception
			Layer& Bottom() const;

			// checks if any active layer is owned by given owner widget. returns true if found, false otherwise
			bool IsExpanded(const Widget* owner) const;
		};
#pragma endregion

	}
}