﻿using UnityEngine;
using System.Collections;
using RAIN.BehaviorTrees;
using RAIN.Core;
using RAIN.Minds;

public class NPC : MonoBehaviour {

	public const int AI_STATE_IDLERANDOM = 0;
	public const int AI_STATE_COMBAT = 1;
	public const int AI_STATE_DYING = 2;
	public const int AI_STATE_STANDING = 3;
	public const int AI_STATE_WALKING = 4 ;

	public string NavMeshRegion;

	public int WhoAmI;

	public int npc_xhome;        //  x coord of home tile
	public int npc_yhome;        //  y coord of home tile
//	public int npc_whoami;       //  npc conversation slot number
	public int npc_hunger;
	public int npc_health;
	public int npc_hp;
	public int npc_arms;          // (not used in uw1)
	public int npc_power;
	public int npc_goal;          // goal that NPC has; 5:kill player 6:? 9:?
	public int npc_attitude;       //attitude; 0:hostile, 1:upset, 2:mellow, 3:friendly
	public int npc_gtarg;         //goal target; 1:player
	public int npc_talkedto;      // is 1 when player already talked to npc
	public int npc_level;
	public int npc_name;       //    (not used in uw1)

	public int state=0; //Set state when not in combat or dying.
	public static UWCharacter playerUW;
	private static bool playerUWReady;
	private GoblinAI Gob;
	private AIRig ai;

	// Use this for initialization
	void Start () {
		Gob = this.GetComponent<GoblinAI>();
		ai = this.GetComponentInChildren<AIRig>();
		ai.AI.Body=this.gameObject;

			playerUW = GameObject.Find ("Gronk").GetComponent<UWCharacter>();

			ai.AI.WorkingMemory.SetItem<GameObject>("playerUW",playerUW.gameObject);
			//ai.AI.IsActive= Vector3.Distance(this.transform.position, playerUW.gameObject.transform.position)<10;

	}
	
	// Update is called once per frame
	void Update () {
		//Gob.isHostile=((npc_attitude==0) && (npc_hp>0));
		//Gob.HP=npc_hp;
		if ( playerUWReady==false)
		{
			if(playerUW!=null)
			{
				ai.AI.WorkingMemory.SetItem<GameObject>("playerUW",playerUW.gameObject);
				playerUWReady=true;
			}
		}
		else
		{
			ai.AI.IsActive= Vector3.Distance(this.transform.position, playerUW.gameObject.transform.position)<10;
			if (ai.AI.IsActive==false)
			{
				return;
			}
			if (npc_hp<0)
			{
				ai.AI.WorkingMemory.SetItem<int>("state",AI_STATE_DYING);//Set to death state.
			//	Debug.Log("NPC Dead");
			}
			else
			{
				if (npc_attitude==0)
					{
					ai.AI.WorkingMemory.SetItem<int>("state",AI_STATE_COMBAT);//Set to combat state.
					}
				else
				{
					ai.AI.WorkingMemory.SetItem<int>("state",state);//Set to idle
				}
			}
		}
	}



	public bool ApplyAttack(int damage)
	{
		//npc_attitude=0;
		npc_attitude=0;
		npc_hp=npc_hp-damage;

		return true;
	}

	public void TalkTo()
	{

		//Debug.Log("Talking to " + WhoAmI) ;
		chains.ActiveControl=3;//Enable UI Elements
		chains.Refresh();

		UITexture portrait = GameObject.Find ("Conversation_Portrait_Right").GetComponent<UITexture>();
		portrait.mainTexture=Resources.Load <Texture2D> ("HUD/PlayerHeads/heads_"+ (playerUW.Body).ToString("0000"));
		
		if (this.WhoAmI<=28)
		{
			//head in charhead.gr
			portrait = GameObject.Find ("Conversation_Portrait_Left").GetComponent<UITexture>();
			portrait.mainTexture=Resources.Load <Texture2D> ("HUD/Charheads/charhead_"+ (WhoAmI-1).ToString("0000"));
			
		}	
		else
		{
			//head in charhead.gr
			int HeadToUse = this.GetComponent<ObjectInteraction>().item_id-64;
			if (HeadToUse >59)
			{
				HeadToUse=0;
			}
			portrait = GameObject.Find ("Conversation_Portrait_Left").GetComponent<UITexture>();
			portrait.mainTexture=Resources.Load <Texture2D> ("HUD/genhead/genhead_"+ (HeadToUse).ToString("0000"));
		}



		Conversation x = (Conversation)this.GetComponent("Conversation_67");
		Conversation.CurrentConversation=WhoAmI;
		Conversation.InConversation=true;
		Conversation.maincam=Camera.main;

		Camera.main.enabled = false;
		StartCoroutine(x.main ());
		//Debug.Log (x.val);
	}
}
