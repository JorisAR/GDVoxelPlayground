extends CharacterBody3D

const MIN_MOVE_SPEED = 0.5;
const MAX_MOVE_SPEED = 512;

@export var camera : Node3D

@export var speed = 5.00
@export var jump_velocity = 4.5
@export var fly_speed : float = 8.0
@export var look_sensitivity : float = 0.1

var input_active : bool = false
@export var is_flying : bool = false

func set_mouse(value: bool) -> void:
	input_active = value;
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED if input_active else Input.MOUSE_MODE_VISIBLE;

func _ready() -> void: 
	set_mouse(true); 

func _input(event) -> void:
	if event.is_action_pressed("change_player_mode"):
		is_flying = !is_flying
	
	if event.is_action_pressed("scroll_up"):
		fly_speed *= 2;
	if event.is_action_pressed("scroll_down"):
		fly_speed *= 0.5;
	fly_speed = clamp(fly_speed, MIN_MOVE_SPEED, MAX_MOVE_SPEED);

	if event is InputEventMouseMotion and input_active:
		rotation.y += -event.relative.x * 0.025 * look_sensitivity;
		camera.rotation.x += -event.relative.y * 0.025 * look_sensitivity;
		camera.rotation.x = clamp(camera.rotation.x, -PI/2.2, PI/2.2)

func _process_flying(_delta: float) -> void:
	var wish_dir_raw = Input.get_vector("move_left", "move_right", "move_forward", "move_backward");
	var direction := (camera.global_basis * Vector3(wish_dir_raw.x, 0, wish_dir_raw.y)).normalized();
	if Input.is_action_pressed("move_up"):
		direction += Vector3.UP;
	if Input.is_action_pressed("move_down"):
		direction += Vector3.DOWN;
	velocity = direction.normalized() * fly_speed

func _process_walking(delta: float) -> void:
	if not is_on_floor():
		velocity += get_gravity() * delta

	if Input.is_action_just_pressed("ui_accept") and is_on_floor():
		velocity.y = jump_velocity
	var input_dir := Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction := (global_basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	if direction:
		velocity.x = direction.x * speed
		velocity.z = direction.z * speed
	else:
		velocity.x = move_toward(velocity.x, 0, speed)
		velocity.z = move_toward(velocity.z, 0, speed)

func _physics_process(delta: float) -> void:
	if is_flying:
		_process_flying(delta)
	else:
		_process_walking(delta)
		
	move_and_slide()
