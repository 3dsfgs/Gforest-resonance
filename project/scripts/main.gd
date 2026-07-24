extends Main

const TitleScreenScene := preload("res://scenes/ui/title_screen.tscn")
const EndingScreenScene := preload("res://scenes/ui/ending_screen.tscn")
const RoomTransitionScript := preload("res://scripts/room_transition.gd")


func _ready() -> void:
	# 流程：标题 → 生日门 → begin_run → 关卡 → 通关结语 → 回标题
	if not run_victory.is_connected(_on_run_victory):
		run_victory.connect(_on_run_victory)
	if not room_advance_requested.is_connected(_on_room_advance_requested):
		room_advance_requested.connect(_on_room_advance_requested)

	var title := TitleScreenScene.instantiate()
	add_child(title)

	if MusicDirector:
		MusicDirector.play_title()


## 不可 override C++ Main.begin_run()；用此入口开战并注入梦房链。
func start_dream_run() -> void:
	if DreamRoomChain:
		configure_run(DreamRoomChain.build_run_config())
	if MusicDirector:
		# 首房固定战斗；后续由 room_transition 按房型切轨。
		MusicDirector.play_for_room_kind("combat")
	begin_run()


func _on_room_advance_requested(next_index: int, hearts: int, room_name: String) -> void:
	if has_node("RoomTransition"):
		return

	var transition: CanvasLayer = RoomTransitionScript.new()
	transition.name = "RoomTransition"
	transition.setup(self, next_index, hearts, room_name)
	add_child(transition)


func _on_run_victory() -> void:
	# 避免重复叠多个结语层
	if has_node("EndingScreen"):
		return

	var ending := EndingScreenScene.instantiate()
	ending.name = "EndingScreen"
	add_child(ending)


func _on_custom_signal_example(delta_time):
	print("DeltaTime value sent from C++ to GDScript: ", delta_time)
