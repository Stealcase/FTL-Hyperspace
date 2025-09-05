#include "Global.h" // TODO: put this is a11y.hpp
#include <array>
#include <cstdint>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include "FTLGame.h"
// #include "FTLGameELF64.h"
#include "accesskit/include/accesskit.h"

#if (defined(__linux__) || defined(__DragonFly__) || defined(__FreeBSD__) ||   \
     defined(__NetBSD__) || defined(__OpenBSD__))
#define UNIX
#endif

const char WINDOW_TITLE[] = "Hello world";

const accesskit_node_id WINDOW_ROOT_ID = 0;
const accesskit_node_id ENGLISH_NODE_ID = 1;
const accesskit_node_id DEUTSCH_NODE_ID = 2;
const accesskit_node_id FRENCH_NODE_ID = 3;
const accesskit_node_id ITALIAN_NODE_ID = 4;
const accesskit_node_id POLISH_NODE_ID = 5;
const accesskit_node_id BRAZILLIAN_PORTUGUESE_NODE_ID = 6;
const accesskit_node_id RUSSIAN_NODE_ID = 7;
const accesskit_node_id JAPANESE_NODE_ID = 8;
const accesskit_node_id CHINESE_HANZI_NODE_ID = 9;
const accesskit_node_id KOREAN_NODE_ID = 10;
const accesskit_node_id ANNOUNCEMENT_ID = 3;
#define INITIAL_FOCUS ENGLISH_NODE_ID

const accesskit_rect BUTTON_1_RECT = {20.0, 20.0, 100.0, 60.0};

const accesskit_rect BUTTON_2_RECT = {20.0, 60.0, 100.0, 100.0};

const int32_t SET_FOCUS_MSG = 0;
const int32_t DO_DEFAULT_ACTION_MSG = 1;

class window_state {
public:
  accesskit_node_id focus;
  const char *announcement;
  window_state() : focus(ENGLISH_NODE_ID) {};
  window_state(class window_state *state) {
    state->focus = ENGLISH_NODE_ID;
    state->announcement = NULL;
    // state->mutex = a11y_CreateMutex();
  }
  window_state(window_state &&) = default;
  window_state(const window_state &) = default;
  window_state &operator=(window_state &&) = default;
  window_state &operator=(const window_state &) = default;
  ~window_state();

private:
};

accesskit_node *build_button(accesskit_node_id id, const char *label,
                             accesskit_rect rect) {
  accesskit_node *node = accesskit_node_new(ACCESSKIT_ROLE_BUTTON);
  accesskit_node_set_bounds(node, rect);
  accesskit_node_set_label(node, label);
  accesskit_node_add_action(node, ACCESSKIT_ACTION_FOCUS);
  accesskit_node_add_action(node, ACCESSKIT_ACTION_CLICK);
  return node;
}

accesskit_node *build_announcement(const char *text) {
  accesskit_node *node = accesskit_node_new(ACCESSKIT_ROLE_LABEL);
  accesskit_node_set_value(node, text);
  accesskit_node_set_live(node, ACCESSKIT_LIVE_POLITE);
  return node;
}

// Wrapper around platform adapters
class Accesskit_FTL_Adapter {

public:
  static Accesskit_FTL_Adapter *GetInstance() { return instance; }
#if defined(__APPLE__)
  accesskit_macos_subclassing_adapter *adapter;
#elif defined(UNIX)
  accesskit_unix_adapter *adapter;
#elif defined(_WIN32)
  accesskit_windows_subclassing_adapter *adapter;
#endif
  Accesskit_FTL_Adapter(
      accesskit_activation_handler_callback activation_handler,
      void *activation_handler_userdata,
      accesskit_action_handler_callback action_handler,
      void *action_handler_userdata,
      accesskit_deactivation_handler_callback deactivation_handler,
      void *deactivation_handler_userdata) {
#if defined(__APPLE__)
    // TODO: add support for MAC without needing wmInfo
    accesskit_macos_add_focus_forwarder_to_window_class("FTLWindow");
    // SDL_SysWMinfo wmInfo;
    // SDL_VERSION(&wmInfo.version);
    // SDL_GetWindowWMInfo(window, &wmInfo);
    // adapter->adapter = accesskit_macos_subclassing_adapter_for_window(
    //     (void *)wmInfo.info.cocoa.window, activation_handler,
    //     activation_handler_userdata, action_handler,
    //     action_handler_userdata);
#elif defined(UNIX)
    adapter = accesskit_unix_adapter_new(
        activation_handler, activation_handler_userdata, action_handler,
        action_handler_userdata, deactivation_handler,
        deactivation_handler_userdata);
#elif defined(_WIN32)
    // SDL_SysWMinfo wmInfo;
    // SDL_VERSION(&wmInfo.version);
    // SDL_GetWindowWMInfo(window, &wmInfo);
    // TODO: Add support for windows without needing WmInfo
    adapter->adapter = accesskit_windows_subclassing_adapter_new(
        wmInfo.info.win.window, activation_handler, activation_handler_userdata,
        action_handler, action_handler_userdata);
#endif
  };
  // Build tree again after button press
  static accesskit_tree_update *
  build_tree_update_for_button_press(void *userdata) {
    window_state *state = (window_state *)
        userdata; // TODO: Fix this cast, this is probably not correct to cast
                  // random pointer to windowstate pointer
    accesskit_node *announcement = build_announcement(state->announcement);
    accesskit_tree_update *update =
        accesskit_tree_update_with_capacity_and_focus(2, state->focus);
    return update;
  }
  Accesskit_FTL_Adapter(Accesskit_FTL_Adapter &&) = default;
  Accesskit_FTL_Adapter(const Accesskit_FTL_Adapter &) = default;
  Accesskit_FTL_Adapter &operator=(Accesskit_FTL_Adapter &&) = default;
  Accesskit_FTL_Adapter &operator=(const Accesskit_FTL_Adapter &) = default;
  ~Accesskit_FTL_Adapter() {
    if (adapter != NULL) {
#if defined(__APPLE__)
      accesskit_macos_subclassing_adapter_free(adapter->adapter);
#elif defined(UNIX)
      accesskit_unix_adapter_free(adapter);
#elif defined(_WIN32)
      accesskit_windows_subclassing_adapter_free(adapter->adapter);
#endif
    }
  }
  static accesskit_node *window_state_build_root() {
    accesskit_node *node = accesskit_node_new(ACCESSKIT_ROLE_WINDOW);
    accesskit_node_set_label(node, WINDOW_TITLE);

    // if (state->announcement != NULL) {
    //   accesskit_node_push_child(node, ANNOUNCEMENT_ID);
    // }
    return node;
  }
  // Call this whenever you want build the tree based on an updated focus
  static accesskit_tree_update *
  build_tree_update_for_focus_update(void *userdata) {
    struct window_state *state =
        (window_state *)userdata; // TODO: Fix unholy cast
    accesskit_tree_update *update =
        accesskit_tree_update_with_focus(state->focus);
    return update;
  }
  // Call this to manually set the focus to a spesific node
  void window_state_set_focus(class window_state *state,
                              accesskit_node_id focus) {
    state->focus = focus;
    accesskit_FTL_adapter_update_if_active(
        this, build_tree_update_for_focus_update, state);
  }
  //
  static void accesskit_FTL_adapter_update_if_active(
      Accesskit_FTL_Adapter *adapter,
      accesskit_tree_update_factory update_factory,
      void *update_factory_userdata) {
#if defined(__APPLE__)
    accesskit_macos_queued_events *events =
        accesskit_macos_subclassing_adapter_update_if_active(
            adapter->adapter, update_factory, update_factory_userdata);
    if (events != NULL) {
      accesskit_macos_queued_events_raise(events);
    }
#elif defined(UNIX)
    accesskit_unix_adapter_update_if_active(adapter->adapter, update_factory,
                                            update_factory_userdata);
#elif defined(_WIN32)
    accesskit_windows_queued_events *events =
        accesskit_windows_subclassing_adapter_update_if_active(
            adapter->adapter, update_factory, update_factory_userdata);
    if (events != NULL) {
      accesskit_windows_queued_events_raise(events);
    }
#endif
  }

private:
  static Accesskit_FTL_Adapter *instance;
};

// Set the window focus state
void accesskit_a11y_adapter_update_window_focus_state(
    Accesskit_FTL_Adapter *adapter, bool is_focused) {
#if defined(__APPLE__)
  accesskit_macos_queued_events *events =
      accesskit_macos_subclassing_adapter_update_view_focus_state(
          adapter->adapter, is_focused);
  if (events != NULL) {
    accesskit_macos_queued_events_raise(events);
  }
#elif defined(UNIX)
  accesskit_unix_adapter_update_window_focus_state(adapter->adapter,
                                                   is_focused);
#endif
  /* On Windows, the subclassing adapter takes care of this. */
}

// When total window size is updated
void accesskit_a11y_adapter_update_root_window_bounds(
    struct Accesskit_FTL_Adapter *adapter, WindowFrame *window) {
#if defined(UNIX)
  int x, y, width, height;
  // a11y_GetWindowPosition(window, &x, &y);
  // a11y_GetWindowSize(window, &width, &height);
  int top, left, bottom, right;
  //  a11y_GetWindowBordersSize(window, &top, &left, &bottom, &right);
  // TODO: Calculate rect for window + outer window of game
  // accesskit_rect outer_bounds = {x - left, y - top, x + width + right,
  //                                y + height + bottom};
  // accesskit_rect inner_bounds = {x, y, x + width, y + height};
  // accesskit_unix_adapter_set_root_window_bounds(adapter->adapter,
  // outer_bounds,
  //                                               inner_bounds);
#endif
}

struct action_handler_state {
  uint32_t event_type; // THis can be a keypress or something else I think
  uint32_t window_id;  // THis is the button or associated interactable element
                       // you want to perform something on
};

void do_action(accesskit_action_request *request, void *userdata) {
  struct action_handler_state *state = (action_handler_state *)
      userdata; // TODO: No idea why this works, think about this
  // event.type = state->event_type;
  // event.user.windowID = state->window_id;
  // event.user.data1 = (void *)((uintptr_t)(request->target));
  if (request->action == ACCESSKIT_ACTION_FOCUS) {
    // event.user.code = SET_FOCUS_MSG;
    // Set the Button to bHover and MAYBE bSelected?
    // SDL_PushEvent(event); // In SDL, this is where we push the event to the
    // system
  } else if (request->action == ACCESSKIT_ACTION_CLICK) {
    // event.user.code = DO_DEFAULT_ACTION_MSG;
    // Click button
    //  SDL_PushEvent(&event); ///
  }
  accesskit_action_request_free(request);
}

void deactivate_accessibility(void *userdata) {
  /* There's nothing in the state that depends on whether the adapter
     is active, so there's nothing to do here. */
}

// TODO: Figure out which hook deals with game focus
void GameGainedFocus(Accesskit_FTL_Adapter adapter) {
  accesskit_a11y_adapter_update_window_focus_state(&adapter, true);
}
// TODO: Figure out which hook deals with game focus
void GameLostFocus(Accesskit_FTL_Adapter adapter) {
  accesskit_a11y_adapter_update_window_focus_state(&adapter, false);
}
void GameWindowSizeChanged(Accesskit_FTL_Adapter adapter) {
  // case SDL_WINDOWEVENT_MAXIMIZED:
  // case SDL_WINDOWEVENT_MOVED:
  // case SDL_WINDOWEVENT_RESIZED:
  // case SDL_WINDOWEVENT_RESTORED:
  // case SDL_WINDOWEVENT_SIZE_CHANGED:
  // case SDL_WINDOWEVENT_SHOWN:
  // accesskit_a11y_adapter_update_root_window_bounds(&adapter, window);
}
// TODO: Figure out how to channel keypresses through here

void NavKeyPressed() {
  // TODO: Simply increment the focus?
  // accesskit_node_id new_focus = (state.focus == ENGLISH) ? DEUTSCH : ENGLISH;
  // window_state_set_focus(&state, &adapter, new_focus);
}
void SelectKeyPressed(window_state state, Accesskit_FTL_Adapter adapter) {
  u_int64_t id = 0; // Make this the current element
  // window_state_press_button(&state, &adapter, id);
}
bool startup_accesskit() {
  window_state state;
  // Uint32 window_id = SDL_GetWindowID(window); // WHy is windowId important
  // here?
  struct action_handler_state action_handler = {
      1, 0}; // Window ID is only relevant for SDL, not Accesskit
  // TODO: Feed in a windovFrame here
  // accesskit_FTL_adapter_init(&adapter, window, build_initial_tree,
  // &state,
  //                            do_action, &action_handler,
  //                            deactivate_accessibility, &state);
}

HOOK_METHOD(LanguageChooser, OnRender, ()->void) {
  LOG_HOOK("HOOK_METHOD -> LanguageChooser::OnRender -> Begin "
           "(A11y.cpp)\n")

  WindowFrame *window;
  // Node and tree is constructed separately
  // root is the root node. tree is the structure that contains nodes.
  // Tree might just be a list of IDs, while node can have children
  accesskit_node *root = Accesskit_FTL_Adapter::window_state_build_root();
  accesskit_tree *tree = accesskit_tree_new(WINDOW_ROOT_ID);
  const std::size_t numLanguages = Global_OptionsScreen_languageList->size();
  accesskit_tree_update *result = accesskit_tree_update_with_capacity_and_focus(
      numLanguages + 1, 1); // Hacky way to focus first element of list. Also,
                            // we are counting all nodes in tree, including root
  accesskit_tree_update_set_tree(result, tree);
  accesskit_tree_update_push_node(result, WINDOW_ROOT_ID, root);
  std::array<accesskit_node_id, numLanguages> accesskit_nodeids;
  auto oldLanguage = G_->GetTextLibrary()->currentLanguage;
  double x_cord = 496.0;
  double y_cord = 360.0 - static_cast<double>(numLanguages) * 37 /
                              2; // TODO: double division might be bad here
  // std::string background;
  double drawHeight;
  for (std::size_t i = 0; i < numLanguages; ++i) {
    accesskit_rect cur_rect;
    if (i == 0) {
      drawHeight = 46;
      cur_rect = {x_cord, y_cord, x_cord + 80, y_cord + drawHeight};
      // background = "optionsUI/language_top.png";
    } else if (i < numLanguages - 1) {
      // background = "optionsUI/language_mid.png";
      drawHeight = 37;
      cur_rect = {x_cord, y_cord, x_cord + 80, y_cord + drawHeight};
    }
    y_cord += drawHeight;
    // G_->GetResources()->RenderImageString(background, drawPoint.x,
    // drawPoint.y, 0, COLOR_WHITE, 1.f, false);
    // G_->GetTextLibrary()->SetLanguage((*Global_OptionsScreen_languageList)[i]);
    // this->buttons[i]->OnRender();
    accesskit_node_id node_id = i + 1;
    std::string label = this->buttons[i]->label->data;
    const char *c_label = label.c_str();
    accesskit_node *node = build_button(node_id, c_label, cur_rect);
    accesskit_node_push_child(node, node_id);
    accesskit_tree_update_push_node(result, node_id, node);
    accesskit_nodeids[i] = node_id;
  }
  accesskit_node_set_children(root, numLanguages, accesskit_nodeids);

  // Accesskit_FTL_Adapter *adapter = new Accesskit_FTL_Adapter(
  //     build_initial_tree, &state, do_action, &action_handler,
  //     deactivate_accessibility, &state);
}

#define a11y (Accesskit_FTL_Adapter::GetInstance())
