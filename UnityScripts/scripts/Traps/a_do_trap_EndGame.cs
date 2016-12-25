﻿using UnityEngine;
using System.Collections;

/// <summary>
/// Ends the game. Plays the ending cutscene.
/// </summary>
public class a_do_trap_EndGame : trap_base {

	public override void ExecuteTrap (int triggerX, int triggerY, int State)
	{
		//base.ExecuteTrap (triggerX, triggerY, State);
		Cutscene_EndGame ce = UWHUD.instance.gameObject.AddComponent<Cutscene_EndGame>();
		UWHUD.instance.CutScenesFull.cs=ce;
		UWHUD.instance.CutScenesFull.Begin();

	}

	public override void PostActivate ()
	{
		//Stop camera from destroying itself
	}
}