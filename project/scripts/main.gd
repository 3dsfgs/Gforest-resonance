extends Main

const BirthdayGateScene := preload("res://scenes/ui/birthday_gate.tscn")


func _ready() -> void:
	# C++ Main::_ready 只做默认设置，不进关；这里弹出生日门。
	var gate := BirthdayGateScene.instantiate()
	add_child(gate)


func _on_custom_signal_example(delta_time):
	print("DeltaTime value sent from C++ to GDScript: ", delta_time)
