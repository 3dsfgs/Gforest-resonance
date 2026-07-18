extends Main

const TitleScreenScene := preload("res://scenes/ui/title_screen.tscn")


func _ready() -> void:
	# 流程：标题 → 生日门 → Main.begin_run() → 关卡
	var title := TitleScreenScene.instantiate()
	add_child(title)


func _on_custom_signal_example(delta_time):
	print("DeltaTime value sent from C++ to GDScript: ", delta_time)
