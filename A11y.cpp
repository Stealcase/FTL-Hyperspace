#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

#include "accesskit/include/accesskit.h"

#if (defined(__linux__) || defined(__DragonFly__) || defined(__FreeBSD__) ||   \
     defined(__NetBSD__) || defined(__OpenBSD__))
#define UNIX
#endif

const char WINDOW_TITLE[] = "Hello world";

const accesskit_node_id WINDOW_ID = 0;
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
#define INITIAL_FOCUS BUTTON_1_ID

const accesskit_rect BUTTON_1_RECT = {20.0, 20.0, 100.0, 60.0};

const accesskit_rect BUTTON_2_RECT = {20.0, 60.0, 100.0, 100.0};

const int32_t SET_FOCUS_MSG = 0;
const int32_t DO_DEFAULT_ACTION_MSG = 1;

accesskit_node *build_button(accesskit_node_id id, const char *label) {
  accesskit_rect rect;
  if (id == ENGLISH) {
    rect = BUTTON_1_RECT;
  } else {
    rect = BUTTON_2_RECT;
  }

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

struct accesskit_a11y_adapter {
#if defined(__APPLE__)
  accesskit_macos_subclassing_adapter *adapter;
#elif defined(UNIX)
  accesskit_unix_adapter *adapter;
#elif defined(_WIN32)
  accesskit_windows_subclassing_adapter *adapter;
#endif
};

void accesskit_a11y_adapter_init(
    struct accesskit_a11y_adapter *adapter, SDL_Window *window,
    accesskit_activation_handler_callback activation_handler,
    void *activation_handler_userdata,
    accesskit_action_handler_callback action_handler,
    void *action_handler_userdata,
    accesskit_deactivation_handler_callback deactivation_handler,
    void *deactivation_handler_userdata) {
#if defined(__APPLE__)
  accesskit_macos_add_focus_forwarder_to_window_class("a11yWindow");
  a11y_SysWMinfo wmInfo;
  a11y_VERSION(&wmInfo.version);
  a11y_GetWindowWMInfo(window, &wmInfo);
  adapter->adapter = accesskit_macos_subclassing_adapter_for_window(
      (void *)wmInfo.info.cocoa.window, activation_handler,
      activation_handler_userdata, action_handler, action_handler_userdata);
#elif defined(UNIX)
  adapter->adapter = accesskit_unix_adapter_new(
      activation_handler, activation_handler_userdata, action_handler,
      action_handler_userdata, deactivation_handler,
      deactivation_handler_userdata);
#elif defined(_WIN32)
  a11y_SysWMinfo wmInfo;
  a11y_VERSION(&wmInfo.version);
  a11y_GetWindowWMInfo(window, &wmInfo);
  adapter->adapter = accesskit_windows_subclassing_adapter_new(
      wmInfo.info.win.window, activation_handler, activation_handler_userdata,
      action_handler, action_handler_userdata);
#endif
}

void accesskit_a11y_adapter_destroy(struct accesskit_sdl_adapter *adapter) {
  if (adapter->adapter != NULL) {
#if defined(__APPLE__)
    accesskit_macos_subclassing_adapter_free(adapter->adapter);
#elif defined(UNIX)
    accesskit_unix_adapter_free(adapter->adapter);
#elif defined(_WIN32)
    accesskit_windows_subclassing_adapter_free(adapter->adapter);
#endif
  }
}

void accesskit_a11y_adapter_update_if_active(
    struct accesskit_a11y_adapter *adapter,
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

void accesskit_a11y_adapter_update_window_focus_state(
    struct accesskit_a11y_adapter *adapter, bool is_focused) {
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

void accesskit_a11y_adapter_update_root_window_bounds(
    struct accesskit_a11y_adapter *adapter, SDL_Window *window) {
#if defined(UNIX)
  int x, y, width, height;
  a11y_GetWindowPosition(window, &x, &y);
  a11y_GetWindowSize(window, &width, &height);
  int top, left, bottom, right;
  a11y_GetWindowBordersSize(window, &top, &left, &bottom, &right);
  accesskit_rect outer_bounds = {x - left, y - top, x + width + right,
                                 y + height + bottom};
  accesskit_rect inner_bounds = {x, y, x + width, y + height};
  accesskit_unix_adapter_set_root_window_bounds(adapter->adapter, outer_bounds,
                                                inner_bounds);
#endif
}

struct window_state {
  accesskit_node_id focus;
  const char *announcement;
  a11y_mutex *mutex;
};

void window_state_init(struct window_state *state) {
  state->focus = INITIAL_FOCUS;
  state->announcement = NULL;
  state->mutex = a11y_CreateMutex();
}

void window_state_destroy(struct window_state *state) {
  a11y_DestroyMutex(state->mutex);
}

void window_state_lock(struct window_state *state) {
  a11y_LockMutex(state->mutex);
}

void window_state_unlock(struct window_state *state) {
  a11y_UnlockMutex(state->mutex);
}

accesskit_node *window_state_build_root(const struct window_state *state) {
  accesskit_node *node = accesskit_node_new(ACCESSKIT_ROLE_WINDOW);
    accesskit_node_push_child(node, ENGLISH_NODE_ID);
    accesskit_node_push_child(node, DEUTSCH_NODE_ID);
    accesskit_node_push_child(node, FRENCH_NODE_ID);
    accesskit_node_push_child(node, ITALIAN_NODE_ID);
    accesskit_node_push_child(node, POLISH_NODE_ID);
    accesskit_node_push_child(node, BRAZILLIAN_PORTUGUESE_NODE_ID);
    accesskit_node_push_child(node, RUSSIAN_NODE_ID);
    accesskit_node_push_child(node, JAPANESE_NODE_ID);
    accesskit_node_push_child(node, CHINESE_HANZI_NODE_ID);
    accesskit_node_push_child(node, KOREAN_NODE_ID);

  if (state->announcement != NULL) {
    accesskit_node_push_child(node, ANNOUNCEMENT_ID);
  }
  accesskit_node_set_label(node, WINDOW_TITLE);
  return node;
}

accesskit_tree_update *
window_state_build_initial_tree(const struct window_state *state) {
  accesskit_node *root = window_state_build_root(state);
  accesskit_node *button_1 = build_button(ENGLISH_NODE_ID, "English");
  accesskit_node *button_2 = build_button(DEUTSCH_NODE_ID, "Deutsch");

  accesskit_node *button_3 = build_button(FRENCH_NODE_ID,"French" ;
  accesskit_node *button_4 = build_button(ITALIAN_NODE_ID,"Italian" ;
  accesskit_node *button_5 = build_button(POLISH_NODE_ID,"Polish" ;
  accesskit_node *button_6 = build_button(BRAZILLIAN_PORTUGUESE_NODE_ID,"Brazillian_portuguese" ;
  accesskit_node *button_7 = build_button(RUSSIAN_NODE_ID,"Russian" ;
  accesskit_node *button_8 = build_button(JAPANESE_NODE_ID,"Japanese" ;
  accesskit_node *button_9 = build_button(CHINESE_HANZI_NODE_ID,"Chinese_hanzi" ;
  accesskit_node *button_10 = build_button(KOREAN_NODE_ID,"korean" ;
  accesskit_tree_update *result = accesskit_tree_update_with_capacity_and_focus(
      (state->announcement != NULL) ? 4 : 3, state->focus);
  accesskit_tree *tree = accesskit_tree_new(WINDOW_ID);
  accesskit_tree_update_set_tree(result, tree);
  accesskit_tree_update_push_node(result, WINDOW_ID, root);
  accesskit_tree_update_push_node(result, ENGLISH_NODE_ID, button_1);
  accesskit_tree_update_push_node(result, DEUTSCH_NODE_ID, button_2);
  if (state->announcement != NULL) {
    accesskit_node *announcement = build_announcement(state->announcement);
    accesskit_tree_update_push_node(result, ANNOUNCEMENT_ID, announcement);
  }
  return result;
}

accesskit_tree_update *build_tree_update_for_button_press(void *userdata) {
  struct window_state *state = userdata;
  accesskit_node *announcement = build_announcement(state->announcement);
  accesskit_node *root = window_state_build_root(state);
  accesskit_tree_update *update =
      accesskit_tree_update_with_capacity_and_focus(2, state->focus);
  accesskit_tree_update_push_node(update, ANNOUNCEMENT_ID, announcement);
  accesskit_tree_update_push_node(update, WINDOW_ID, root);
  return update;
}

void window_state_press_button(struct window_state *state,
                               struct accesskit_a11y_adapter *adapter,
                               accesskit_node_id id) {
  const char *text;
  if (id == ENGLISH) {
    text = "You pressed button 1";
  } else {
    text = "You pressed button 2";
  }
  state->announcement = text;
  accesskit_a11y_adapter_update_if_active(
      adapter, build_tree_update_for_button_press, state);
}

accesskit_tree_update *build_tree_update_for_focus_update(void *userdata) {
  struct window_state *state = userdata;
  accesskit_tree_update *update =
      accesskit_tree_update_with_focus(state->focus);
  return update;
}

void window_state_set_focus(struct window_state *state,
                            struct accesskit_a11y_adapter *adapter,
                            accesskit_node_id focus) {
  state->focus = focus;
  accesskit_a11y_adapter_update_if_active(
      adapter, build_tree_update_for_focus_update, state);
}

struct action_handler_state {
  Uint32 event_type;
  Uint32 window_id;
};

void do_action(accesskit_action_request *request, void *userdata) {
  struct action_handler_state *state = userdata;
  a11y_Event event;
  a11y_zero(event);
  event.type = state->event_type;
  event.user.windowID = state->window_id;
  event.user.data1 = (void *)((uintptr_t)(request->target));
  if (request->action == ACCESSKIT_ACTION_FOCUS) {
    event.user.code = SET_FOCUS_MSG;
    a11y_PushEvent(&event);
  } else if (request->action == ACCESSKIT_ACTION_CLICK) {
    event.user.code = DO_DEFAULT_ACTION_MSG;
    a11y_PushEvent(&event);
  }
  accesskit_action_request_free(request);
}

accesskit_tree_update *build_initial_tree(void *userdata) {
  struct window_state *state = userdata;
  window_state_lock(state);
  accesskit_tree_update *update = window_state_build_initial_tree(state);
  window_state_unlock(state);
  return update;
}

void deactivate_accessibility(void *userdata) {
  /* There's nothing in the state that depends on whether the adapter
     is active, so there's nothing to do here. */
}

int main(int argc, char *argv[]) {
  printf("This example has no visible GUI, and a keyboard interface:\n");
  printf("- [Tab] switches focus between two logical buttons.\n");
  printf("- [Space] 'presses' the button, adding static text in a live region "
         "announcing that it was pressed.\n");
#if defined(_WIN32)
  printf("Enable Narrator with [Win]+[Ctrl]+[Enter] (or [Win]+[Enter] on older "
         "versions of Windows).\n");
#elif defined(UNIX)
  printf("Enable Orca with [Super]+[Alt]+[S].\n");
#endif
  if (a11y_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "a11y initialization failed: (%s)\n", SDL_GetError());
    return -1;
  }
  Uint32 user_event = a11y_RegisterEvents(1);
  if (user_event == (Uint32)-1) {
    fprintf(stderr, "Couldn't register user event: (%s)\n", a11y_GetError());
    return -1;
  }

  struct window_state state;
  window_state_init(&state);
  a11y_Window *window =
      a11y_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                        a11y_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
  a11y_Surface *screenSurface = SDL_GetWindowSurface(window);
  a11y_FillRect(screenSurface, NULL,
                a11y_MapRGB(&(*screenSurface->format), 0x00, 0x00, 0x00));
  a11y_UpdateWindowSurface(window);
  Uint32 window_id = a11y_GetWindowID(window);
  struct action_handler_state action_handler = {user_event, window_id};
  struct accesskit_a11y_adapter adapter;
  accesskit_a11y_adapter_init(&adapter, window, build_initial_tree, &state,
                              do_action, &action_handler,
                              deactivate_accessibility, &state);
  a11y_ShowWindow(window);

  a11y_Event event;
  while (a11y_WaitEvent(&event)) {
    if (event.type == a11y_QUIT) {
      break;
    } else if (event.type == a11y_WINDOWEVENT &&
               event.window.windowID == window_id) {
      switch (event.window.event) {
      case a11y_WINDOWEVENT_FOCUS_GAINED:
        accesskit_a11y_adapter_update_window_focus_state(&adapter, true);
        break;
      case a11y_WINDOWEVENT_FOCUS_LOST:
        accesskit_a11y_adapter_update_window_focus_state(&adapter, false);
        break;
      case a11y_WINDOWEVENT_MAXIMIZED:
      case a11y_WINDOWEVENT_MOVED:
      case a11y_WINDOWEVENT_RESIZED:
      case a11y_WINDOWEVENT_RESTORED:
      case a11y_WINDOWEVENT_SIZE_CHANGED:
      case a11y_WINDOWEVENT_SHOWN:
        accesskit_a11y_adapter_update_root_window_bounds(&adapter, window);
        break;
      }
    } else if (event.type == a11y_KEYDOWN && event.key.windowID == window_id) {
      switch (event.key.keysym.sym) {
      case a11yK_TAB:
        window_state_lock(&state);
        accesskit_node_id new_focus =
            (state.focus == ENGLISH) ? DEUTSCH : ENGLISH;
        window_state_set_focus(&state, &adapter, new_focus);
        window_state_unlock(&state);
        break;
      case a11yK_SPACE:
        window_state_lock(&state);
        accesskit_node_id id = state.focus;
        window_state_press_button(&state, &adapter, id);
        window_state_unlock(&state);
        break;
      }
    } else if (event.type == user_event && event.user.windowID == window_id) {
      accesskit_node_id target =
          (accesskit_node_id)((uintptr_t)(event.user.data1));
      if (target == ENGLISH || target == DEUTSCH) {
        window_state_lock(&state);
        if (event.user.code == SET_FOCUS_MSG) {
          window_state_set_focus(&state, &adapter, target);
        } else if (event.user.code == DO_DEFAULT_ACTION_MSG) {
          window_state_press_button(&state, &adapter, target);
        }
        window_state_unlock(&state);
      }
    }
  }

  accesskit_a11y_adapter_destroy(&adapter);
  window_state_destroy(&state);
  a11y_Quit();
  return 0;
}
