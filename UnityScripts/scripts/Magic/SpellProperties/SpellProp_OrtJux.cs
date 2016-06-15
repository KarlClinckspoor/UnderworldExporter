﻿using UnityEngine;
using System.Collections;

public class SpellProp_OrtJux : SpellProp {
	//Properties for the Ort Jux spell.

	public override void init ()
	{
		base.init ();
		ProjectileSprite = "Sprites/objects_023";
		Force=200.0f;
		BaseDamage=3;
		impactFrameStart=46;
		impactFrameEnd=50;
		spread=0;
		noOfCasts=1;
	}
}