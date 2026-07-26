extends Main

const TitleScreenScene := preload("res://scenes/ui/title_screen.tscn")
const EndingScreenScene := preload("res://scenes/ui/ending_screen.tscn")
const StorySequenceScene := preload("res://scenes/prefabs/story_sequence.tscn")
const BedroomAnchorScene := preload("res://scenes/prefabs/bedroom_anchor.tscn")
const RoomTransitionScript := preload("res://scripts/room_transition.gd")
const MonologueHudScript := preload("res://scripts/monologue_hud.gd")
const PauseMenuScript := preload("res://scripts/pause_menu.gd")
const BreadcrumbHudScript := preload("res://scripts/dream_breadcrumb_hud.gd")

const BEDROOM_HOLD_SEC := 1.35

var _in_dream_run := false


func _ready() -> void:
	# 流程：标题 → 生辰档案 → 锚点夜 → OP → 梦房链 → ED → 锚点日 → 结语 → 回标题
	process_mode = Node.PROCESS_MODE_ALWAYS
	if not run_victory.is_connected(_on_run_victory):
		run_victory.connect(_on_run_victory)
	if not room_advance_requested.is_connected(_on_room_advance_requested):
		room_advance_requested.connect(_on_room_advance_requested)

	var title := TitleScreenScene.instantiate()
	title.name = "TitleScreen"
	add_child(title)

	if MusicDirector:
		MusicDirector.play_title()


func _unhandled_input(event: InputEvent) -> void:
	if not event.is_action_pressed("ui_cancel"):
		return
	if not _can_open_pause():
		return
	_open_pause_menu()
	get_viewport().set_input_as_handled()


## 不可 override C++ Main.begin_run()；用此入口开战并注入梦房链。
## 暂停「重开本晚」直接调用：跳过 OP。
func start_dream_run() -> void:
	_in_dream_run = true
	_ensure_monologue_hud()
	_ensure_breadcrumb_hud()
	if DreamRoomChain:
		configure_run(DreamRoomChain.build_run_config())
	var first_kind := "explore"
	var first_name := "雾缘·小径"
	if DreamRoomChain:
		first_kind = DreamRoomChain.get_room_kind_at(0)
		var chain: Array = DreamRoomChain.get_chain()
		if not chain.is_empty():
			first_name = str((chain[0] as Dictionary).get("display_name", first_name))
	if MusicDirector:
		MusicDirector.play_for_room_kind(first_kind)
	begin_run()
	_update_breadcrumb(0)
	# 首房进场独白（无过场卡）
	var mono := get_node_or_null("MonologueHud")
	if mono != null and mono.has_method("on_room_entered"):
		mono.on_room_entered(0, first_name, first_kind)


## 生日门确认后：房间锚点（夜）→ 开场分镜 → 开战。
func play_opening_then_run() -> void:
	if _story_flow_busy():
		return
	_play_bedroom_anchor(false, func():
		play_story_sequence("op", func():
			start_dream_run()
		)
	)


## 暂停「回标题」：卸关卡 + 清梦状态 + 重建标题屏。
func go_to_title() -> void:
	_in_dream_run = false
	_clear_dream_hud()
	if has_method("return_to_title"):
		return_to_title()
	if MusicDirector:
		MusicDirector.play_title()
	_ensure_title_screen()


func play_story_sequence(kind: String, on_finished: Callable = Callable()) -> void:
	if has_node("StorySequence"):
		return
	var seq: CanvasLayer = StorySequenceScene.instantiate()
	seq.name = "StorySequence"
	seq.sequence_key = kind
	seq.load_from_story(kind)
	add_child(seq)
	if on_finished.is_valid():
		seq.finished.connect(on_finished, CONNECT_ONE_SHOT)


func _on_room_advance_requested(next_index: int, hearts: int, room_name: String) -> void:
	if has_node("RoomTransition"):
		return

	var mono := get_node_or_null("MonologueHud")
	if mono != null and mono.has_method("on_room_cleared"):
		var cleared_name := ""
		if DreamRoomChain:
			var chain: Array = DreamRoomChain.get_chain()
			var cleared_i := next_index - 1
			if cleared_i >= 0 and cleared_i < chain.size():
				cleared_name = str((chain[cleared_i] as Dictionary).get("display_name", ""))
		mono.on_room_cleared(next_index - 1, cleared_name)

	var enter_kind := "combat"
	if DreamRoomChain:
		enter_kind = DreamRoomChain.get_room_kind_at(next_index)

	var transition: CanvasLayer = RoomTransitionScript.new()
	transition.name = "RoomTransition"
	transition.setup(self, next_index, hearts, room_name, enter_kind)
	add_child(transition)


## 过场加载下一房后由 room_transition 回调：独白 + 面包屑。
func on_room_transition_entered(room_index: int, room_name: String, room_kind: String) -> void:
	_update_breadcrumb(room_index)
	var mono := get_node_or_null("MonologueHud")
	if mono != null and mono.has_method("on_room_entered"):
		mono.on_room_entered(room_index, room_name, room_kind)


func _on_run_victory() -> void:
	if _story_flow_busy() or has_node("EndingScreen"):
		return

	_in_dream_run = false
	_clear_dream_hud()

	# 卸下关卡，避免 ED 期间仍可操作
	if has_method("return_to_title"):
		return_to_title()

	# ED 分镜 → 锚点日态 → 文字结语
	play_story_sequence("ed", func():
		_play_bedroom_anchor(true, func():
			_show_ending_screen()
		)
	)


func _show_ending_screen() -> void:
	if has_node("EndingScreen"):
		return
	var ending := EndingScreenScene.instantiate()
	ending.name = "EndingScreen"
	add_child(ending)


func _play_bedroom_anchor(is_day: bool, on_finished: Callable) -> void:
	if has_node("BedroomAnchorLayer"):
		return

	var layer := CanvasLayer.new()
	layer.name = "BedroomAnchorLayer"
	layer.layer = 94
	layer.process_mode = Node.PROCESS_MODE_ALWAYS
	add_child(layer)

	var dim := ColorRect.new()
	dim.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	dim.color = Color(0.02, 0.03, 0.05, 1.0)
	layer.add_child(dim)

	var anchor: TextureRect = BedroomAnchorScene.instantiate()
	anchor.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	anchor.modulate.a = 0.0
	if is_day:
		anchor.show_day()
	else:
		anchor.show_night()
	layer.add_child(anchor)

	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	var tw := create_tween()
	tw.tween_property(anchor, "modulate:a", 1.0, 0.45).set_trans(Tween.TRANS_SINE)
	tw.tween_interval(BEDROOM_HOLD_SEC)
	tw.tween_property(anchor, "modulate:a", 0.0, 0.4).set_trans(Tween.TRANS_SINE)
	tw.tween_callback(func():
		layer.queue_free()
		if on_finished.is_valid():
			on_finished.call()
	)


func _ensure_monologue_hud() -> void:
	if has_node("MonologueHud"):
		return
	var hud: CanvasLayer = MonologueHudScript.new()
	hud.name = "MonologueHud"
	add_child(hud)


func _ensure_breadcrumb_hud() -> void:
	if has_node("DreamBreadcrumbHud"):
		return
	var hud: CanvasLayer = BreadcrumbHudScript.new()
	hud.name = "DreamBreadcrumbHud"
	add_child(hud)


func _update_breadcrumb(index: int) -> void:
	var hud := get_node_or_null("DreamBreadcrumbHud")
	if hud != null and hud.has_method("set_index"):
		hud.set_index(index)


func _clear_dream_hud() -> void:
	var mono := get_node_or_null("MonologueHud")
	if mono != null:
		mono.queue_free()
	var crumbs := get_node_or_null("DreamBreadcrumbHud")
	if crumbs != null:
		crumbs.queue_free()


func get_dream_progress_text() -> String:
	var hud := get_node_or_null("DreamBreadcrumbHud")
	var idx := 0
	if hud != null and hud.has_method("get_current_index"):
		idx = int(hud.get_current_index())
	return BreadcrumbHudScript.format_chain_text(idx)


func _story_flow_busy() -> bool:
	return has_node("StorySequence") or has_node("BedroomAnchorLayer")


func _can_open_pause() -> bool:
	if not _in_dream_run:
		return false
	if has_node("PauseMenu"):
		return false
	if has_node("RoomTransition"):
		return false
	if has_node("EndingScreen"):
		return false
	if _story_flow_busy():
		return false
	return true


func _open_pause_menu() -> void:
	var menu: CanvasLayer = PauseMenuScript.new()
	menu.name = "PauseMenu"
	menu.setup(self)
	add_child(menu)


func _ensure_title_screen() -> void:
	if has_node("TitleScreen"):
		return
	for child in get_children():
		# title_screen.gd 动态搭建 UI，根为 CanvasLayer
		if child.get_script() == preload("res://scripts/title_screen.gd"):
			return
	var title := TitleScreenScene.instantiate()
	title.name = "TitleScreen"
	add_child(title)


func _on_custom_signal_example(delta_time):
	print("DeltaTime value sent from C++ to GDScript: ", delta_time)
