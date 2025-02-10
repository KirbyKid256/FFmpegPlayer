extends Node2D

@onready var theora_ogv: VideoStreamPlayer = $TheoraOGV
@onready var ffmpeg_ogv: VideoStreamPlayer = $FFmpegOGV
@onready var ffmpeg_mp4: VideoStreamPlayer = $FFmpegMP4

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_accept"):
		if theora_ogv.is_playing():
			theora_ogv.stop()
			ffmpeg_ogv.stop()
			ffmpeg_mp4.stop()
		else:
			theora_ogv.play()
			ffmpeg_ogv.play()
			ffmpeg_mp4.play()
