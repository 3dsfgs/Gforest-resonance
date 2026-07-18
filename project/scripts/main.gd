extends Main

const TitleScreenScene := preload("res://scenes/ui/title_screen.tscn")
const EndingScreenScene := preload("res://scenes/ui/ending_screen.tscn")


func _ready() -> void:
	# 流程：标题 → 生日门 → begin_run → 关卡 → 通关结语 → 回标题
	if not run_victory.is_connected(_on_run_victory):
		run_victory.connect(_on_run_victory)

	var title := TitleScreenScene.instantiate()
	add_child(title)

	if MusicDirector:
		MusicDirector.play_title()


func _on_run_victory() -> void:
	# 避免重复叠多个结语层
	if has_node("EndingScreen"):
		return

	var ending := EndingScreenScene.instantiate()
	ending.name = "EndingScreen"
	add_child(ending)


func _on_custom_signal_example(delta_time):
	print("DeltaTime value sent from C++ to GDScript: ", delta_time)
