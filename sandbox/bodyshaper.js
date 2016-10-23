var SCREEN_WIDTH = window.innerWidth;
var SCREEN_HEIGHT = window.innerHeight;

var container;

var camera, scene;
var renderer;

//var mesh;

var mouseX = 0, mouseY = 0;

var windowHalfX = window.innerWidth / 2;
var windowHalfY = window.innerHeight / 2;

var gui, skinConfig, morphConfig;

var skincolorParams = { 
	
	Hue:		0.0917,
	Saturation: 1.0,
	Lightness:  0.878
	
	};

var hairParams = {

	Hair: false,
	Hue:		0.121,
	Saturation: 0.73,
	Lightness:  0.66
	
	};

var shapeParams = {

	Height:		0.121,
	Weight: 0.73,
	Waist:  0.66,
	Hip:		0.121,
	Leg: 0.73,
	Shoulder:  0.66,
	Arm: 0.5
	
	
	};
	
var genderParams = { 
	Sex: 'A_male'
	};
	

var material = new THREE.MeshStandardMaterial( { 
				color: 0xFFE3C1,
				roughness: 1.0, 
				metalness: 0.0, 
				shading: THREE.SmoothShading,
				wireframe: false 
				
				} );
					
var male_geometry, male_mesh;

document.addEventListener( 'mousemove', onDocumentMouseMove, false );

init();
animate();

function init() {

	container = document.getElementById( 'container' );

	// RENDERER

	renderer = new THREE.WebGLRenderer( { antialias: true } );
	renderer.setClearColor( 0xffffff );
	renderer.setPixelRatio( window.devicePixelRatio );
	renderer.setSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	container.appendChild( renderer.domElement );
	
	camera = new THREE.PerspectiveCamera( 45, window.innerWidth / window.innerHeight, 1, 100000 );
	camera.position.set( 0, 0, 3 );

	scene = new THREE.Scene();
	scene.add( camera );
	
	// LIGHTS

	var dirLight = new THREE.DirectionalLight( 0xffffff, 1 );
	dirLight.position.set( 0, 140, 500 ).normalize();
	camera.add( dirLight );
	camera.add( dirLight.target );

	// CHARACTER
	loadMale();
	
	gui = new dat.GUI();
	gui.width = Math.round( window.innerWidth/5 );
	setupGenderGUI();
	setupSkinColorGUI();
	setupHairGUI();
	setupMorphsGUI();
	gui.open();

	window.addEventListener( 'resize', onWindowResize, false );

	controls = new THREE.OrbitControls( camera, renderer.domElement );
	
	controls.addEventListener( 'change', render );

}

function removeObjectByName( objectname ) {
	var maleObject = scene.getObjectByName( objectname );
	if( typeof maleObject != "undefined" ) {
		scene.remove( maleObject );
	}	
}

function loadMale() {
		
	var loader = new THREE.PLYLoader();
	loader.load( './MaleFemale_burn/male2.ply', function ( male_geometry ) {
		//geometry.computeFaceNormals();
		male_geometry.computeVertexNormals();

		male_mesh = new THREE.Mesh( male_geometry, material );
		male_mesh.name = "A_male";
		male_mesh.rotation.x = - Math.PI / 2;
		male_mesh.scale.multiplyScalar( 0.001 ); // scale to mm

		scene.add( male_mesh );
	});
}

function loadFemale() {
	
	var loader = new THREE.PLYLoader();
	loader.load( './MaleFemale_burn/female2.ply', function ( female_geometry ) {

		//geometry.computeFaceNormals();
		female_geometry.computeVertexNormals();

		var female_mesh = new THREE.Mesh( female_geometry, material );
		female_mesh.name = "A_female";
		female_mesh.rotation.x = - Math.PI / 2;
		female_mesh.scale.multiplyScalar( 0.001 ); // scale to mm

		scene.add( female_mesh );
	});
	
}

function setupGenderGUI() {

	var GenderGUI = gui.addFolder( "Gender" );

	GenderGUI.add( genderParams, 'Sex', { 'Male': 'A_male', 'Female': 'A_female' } ).onChange( switchSex );
	
	GenderGUI.open();
}

function setupSkinColorGUI() {

	var SkinColorGUI = gui.addFolder( "Skin Tone" );
	
	SkinColorGUI.add( skincolorParams, "Hue", 0.0, 1.0, 0.025 ).name( "Hue" ).onChange( changeSkinColor );
	SkinColorGUI.add( skincolorParams, "Saturation", 0.0, 1.0, 0.025 ).name( "Saturation" ).onChange( changeSkinColor );
	SkinColorGUI.add( skincolorParams, "Lightness", 0.0, 1.0, 0.025 ).name( "Lightness" ).onChange( changeSkinColor );
	
	/*SkinColorGUI.add( 'Skin Tone', material.color.getHex() , function( val ) {

		material.color.setHex( val );
		animate();

	}, true );*/

	SkinColorGUI.open();
}

function setupHairGUI() {

	var HairGUI = gui.addFolder( "Hair" );
	
	HairGUI.add( hairParams, 'Hair').onChange( function() {
	});

	HairGUI.add( hairParams, "Hue", 0.0, 1.0, 0.025 ).name( "Hue" ).onChange( render );
	HairGUI.add( hairParams, "Saturation", 0.0, 1.0, 0.025 ).name( "Saturation" ).onChange( render );
	HairGUI.add( hairParams, "Lightness", 0.0, 1.0, 0.025 ).name( "Lightness" ).onChange( render );
				
	HairGUI.open();

}

function setupMorphsGUI() {

	var MorphGui = gui.addFolder( "Body Measurements" );

	MorphGui.add( shapeParams, "Height", 0.0, 1.0, 0.025 ).name( "Height" ).onChange( render );
	MorphGui.add( shapeParams, "Weight", 0.0, 1.0, 0.025 ).name( "Weight" ).onChange( render );
	MorphGui.add( shapeParams, "Waist", 0.0, 1.0, 0.025 ).name( "Waist" ).onChange( render );

	MorphGui.open();

}

function changeSkinColor() {
	var skinColor = new THREE.Color();
	skinColor.setHSL( skincolorParams.Hue, skincolorParams.Saturation, skincolorParams.Lightness );
	material.color.setHex( skinColor.getHex() );
	
}

function switchSex() {
	if( genderParams.Sex == 'A_male' ) {
	
		removeObjectByName( 'A_female' );
		loadMale();	
	
	}	
	else if( genderParams.Sex == 'A_female' ) {
	
		removeObjectByName( 'A_male' );
		loadFemale();					
	}
}

function onWindowResize() {

	windowHalfX = window.innerWidth / 2;
	windowHalfY = window.innerHeight / 2;

	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();

	renderer.setSize( window.innerWidth, window.innerHeight );

}

function onDocumentMouseMove( event ) {

	mouseX = ( event.clientX - windowHalfX ) * 10;
	mouseY = ( event.clientY - windowHalfY ) * 10;

}

function animate() {

	requestAnimationFrame( animate );

	controls.update();

	render();

}

function render() {
	
	renderer.render( scene, camera );

}