﻿using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using System.ComponentModel;
using static KCS_GUI.CsvDataSet;

namespace KCS_GUI {
	public partial class MainForm : Form {
		/* メンバ変数 */
		// 定数群
		//ソフト名
		const string SoftName = "KanColleSimulator";
		//艦隊数の最大
		const int MaxFleetSize = 2;
		//艦隊における艦船数の最大
		const int MaxUnitSize = 6;
		//制空計算用(艦戦ボーナスと艦爆ボーナス)
		static List<double> bonusPF = new List<double> { 0.0, 0.0, 2.0, 5.0, 9.0, 14.0, 14.0, 22.0 };
		static List<double> bonusWB = new List<double> { 0.0, 0.0, 1.0, 1.0, 1.0, 3.0, 3.0, 6.0 };

		//種別番号→インデックスのリスト変換
		Dictionary<int, List<int>> WeaponTypeToIndexList;
		//艦種番号→インデックスのリスト変換
		Dictionary<int, List<int>> KammusuTypeToIndexList;
		// 画面表示用データ
		//艦隊タブ
		Fleet FormFleet;
		//マップタブ
		MapData FormMapData;
		//艦隊タブにおけるファイルパス
		string FleetFilePath;
		//マップタブにおけるファイルパス
		string MapFilePath;
		//ErrorProvider
		private System.Windows.Forms.ErrorProvider error_provider_level;
		private System.Windows.Forms.ErrorProvider error_provider_luck;
		private System.Windows.Forms.ErrorProvider error_provider_cond;
		/* コンストラクタ */
		public MainForm() {
			InitializeComponent();
			try {
				ReadWeaponData();
				ReadKammusuData();
				RedrawWeaponNameList();
				RedrawKammusuNameList();
				RedrawMapKammusuNameList();
				FormFleet = new Fleet();
				FormMapData = new MapData();
				error_provider_level = new System.Windows.Forms.ErrorProvider();
				error_provider_luck = new System.Windows.Forms.ErrorProvider();
				error_provider_cond = new System.Windows.Forms.ErrorProvider();
			} catch(Exception ex) {
				MessageBox.Show(ex.Message, SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				this.Close();
			}
		}

		/* 各イベント毎の処理 */
		// メニュー
		private void NewFileMenuItem_Click(object sender, EventArgs e) {
			if(MainTabControl.SelectedIndex == 0) {
				FormFleet = new Fleet();
				HQLevelTextBox.Text = FormFleet.level.ToString();
				FleetTypeComboBox.SelectedIndex = FormFleet.type;
				FleetSelectComboBox_SelectedIndexChanged(sender, e);
				RedrawAntiAirScore();
				RedrawSearchPower();
			} else if(MainTabControl.SelectedIndex == 1) {
				FormMapData = new MapData();
				MapPositionListBox.Items.Clear();
				MapPositionListBox.Refresh();
				RedrawMapAntiAirScore();
			}
		}
		private void OpenFileMenuItem_Click(object sender, EventArgs e) {
			if(MainTabControl.SelectedIndex == 0) {
				// ファイルを開くダイアログを表示する
				OpenFileDialog ofd = new OpenFileDialog();
				ofd.Filter = "艦隊データ(*.json)|*.json|すべてのファイル(*.*)|*.*";
				if(ofd.ShowDialog() != DialogResult.OK)
					return;
				FleetFilePath = ofd.FileName;
				// ファイルを詠みこみ、JSONとして解釈する
				Tuple<Fleet, bool> setFleet = ReadJsonFile(FleetFilePath);
				if(!setFleet.Item2)
					return;
				// 読み込んだデータを画面に反映する
				FormFleet = setFleet.Item1;
				HQLevelTextBox.Text = FormFleet.level.ToString();
				FleetTypeComboBox.SelectedIndex = FormFleet.type;
				FleetSelectComboBox_SelectedIndexChanged(sender, e);
				RedrawAntiAirScore();
				RedrawSearchPower();
			} else if(MainTabControl.SelectedIndex == 1) {
				// ファイルを開くダイアログを表示する
				OpenFileDialog ofd = new OpenFileDialog();
				ofd.Filter = "マップデータ(*.map)|*.map|すべてのファイル(*.*)|*.*";
				if(ofd.ShowDialog() != DialogResult.OK)
					return;
				MapFilePath = ofd.FileName;
				// ファイルを詠みこみ、マップデータのJSONとして解釈する
				Tuple<MapData, bool> setMapData = ReadMapFile(MapFilePath);
				if(!setMapData.Item2)
					return;
				// 読み込んだデータを画面に反映する
				FormMapData = setMapData.Item1;
				MapPositionListBox.Items.Clear();
				foreach(var position in FormMapData.position) {
					MapPositionListBox.Items.Add(position.name);
				}
				MapPositionListBox.Refresh();
				RedrawMapAntiAirScore();
			}
			return;
		}
		private bool CreateFleetFile() {
			// セーブデータを作成する
			string saveData = FormFleet.ToJson();
			// 作成したデータを保存する
			var sfd = new SaveFileDialog();
			sfd.Filter = "艦隊データ(*.json)|*.json|すべてのファイル(*.*)|*.*";
			if (sfd.ShowDialog() != DialogResult.OK)
				return false;
			this.FleetFilePath = sfd.FileName;
			return true;
		}
		private void SaveFleetFile(bool force_create = false) {
			// 事前チェック
			if (FormFleet.unit[0].Count == 0) {
				MessageBox.Show("艦娘を第1艦隊に1隻以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if ((FleetFilePath == null || FleetFilePath == "" || force_create) && !CreateFleetFile()) return;
			// セーブデータを作成する
			string saveData = FormFleet.ToJson();
			// 作成したデータを保存する
			var sw = new StreamWriter(FleetFilePath, false, Encoding.GetEncoding("shift-jis"));
			sw.Write(saveData);
			sw.Close();
		}
		private bool CreateMapFile() {
			// セーブデータを作成する
			string saveData = FormMapData.ToJson();
			// 作成したデータを保存する
			var sfd = new SaveFileDialog();
			sfd.Filter = "マップデータ(*.map)|*.map|すべてのファイル(*.*)|*.*";
			if (sfd.ShowDialog() != DialogResult.OK)
				return false;
			this.MapFilePath = sfd.FileName;
			return true;
		}
		private void SaveMapFile(bool force_create = false) {
			// 事前チェック
			if (FormMapData.position.Count == 0) {
				MessageBox.Show("パターンをマスに1つ以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if (FormMapData.position[0].fleet.Count == 0) {
				MessageBox.Show("艦隊をパターンに1つ以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if (FormMapData.position[0].fleet[0].unit[0].Count == 0) {
				MessageBox.Show("艦娘を艦隊に1隻以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if ((MapFilePath == null || MapFilePath == "" || force_create) && !CreateMapFile()) return;
			// セーブデータを作成する
			string saveData = FormMapData.ToJson();
			// 作成したデータを保存する
			var sw = new StreamWriter(MapFilePath, false, Encoding.GetEncoding("shift-jis"));
			sw.Write(saveData);
			sw.Close();
		}
		private void SaveSFileMenuItem_Click(object sender, EventArgs e) {
			if(MainTabControl.SelectedIndex == 0) {
				SaveFleetFile();
			} else if(MainTabControl.SelectedIndex == 1) {
				SaveMapFile();
			}
		}
		private void SaveAFileMenuItem_Click(object sender, EventArgs e) {
			if(MainTabControl.SelectedIndex == 0) {
				SaveFleetFile(true);
			}
			else if(MainTabControl.SelectedIndex == 1) {
				SaveMapFile(true);
			}
		}
		private void ExitMenuItem_Click(object sender, EventArgs e) {
			this.Close();
		}
		//バージョン情報表示
		private void VersionInfoMenuItem_Click(object sender, EventArgs e) {
			Assembly assem = Assembly.GetExecutingAssembly();
			Version v = assem.GetName().Version;
			string version = v.Major.ToString() + "." + v.Minor.ToString() + "." + v.Build.ToString() + "." + v.Revision.ToString();
			string assemblyProduct = ((AssemblyProductAttribute)Attribute.GetCustomAttribute(assem, typeof(AssemblyProductAttribute))).Product;
			string assemblyCopyright = ((AssemblyCopyrightAttribute)Attribute.GetCustomAttribute(assem, typeof(AssemblyCopyrightAttribute))).Copyright;
			string verInfo
				= assemblyProduct + ".exe\n"
				+ "バージョン：" + version + "\n"
				+ "作成者：YSR\n"
				+ assemblyCopyright;
			MessageBox.Show(verInfo, SoftName);
		}

		// 艦娘エディタタブ
		private void AddKammusuButton_Click(object sender, EventArgs e) {
			if(KammusuTypeComboBox.SelectedIndex == -1
			|| KammusuNameComboBox.SelectedIndex == -1
			|| FleetSelectComboBox.SelectedIndex == -1)
				return;
			// 1艦隊には6隻まで
			if(FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count == MaxUnitSize)
				return;
			// 艦娘データを作成する
			int index = KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex];
			var id = data.Ships[index].艦船ID;
			var level = KammusuLevelTextBox.Text.ParseInt();
			var luck = KammusuLuckTextBox.Text.ParseInt();
			var cond = KammusuCondTextBox.Text.ParseInt();
			var setKammusu = new Kammusu(id, level, luck, cond);
			// 作成した艦娘データを追加する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].Add(setKammusu);
			KammusuSelectListBox.Items.Add(setKammusu.艦名);
			KammusuSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		static private bool IsInRange(int val, int min, int max) {
			return (min <= val && val <= max);
		}
		static private bool IsVaidIndex(int val, int size) {
			return IsInRange(val, 0, size - 1);
		}
		private void KammusuLevelTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int level = int.Parse(KammusuLevelTextBox.Text);
				if (level < 1 || 155 < level) {
					e.Cancel = true;
					error_provider_level.SetError(KammusuLevelTextBox, "1-255の値を入力してください");//range error
				}
				else {
					error_provider_level.SetError(KammusuLevelTextBox, "");//error clear
				}
			}
			catch (Exception) {
				e.Cancel = true;
				error_provider_level.SetError(KammusuLevelTextBox, "整数値以外の文字が入力されました！！");
			}
		}
		private void KammusuLevelTextBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
			)
				FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].lv = int.Parse(KammusuLevelTextBox.Text);
		}
		private void KammusuLuckTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int luck = int.Parse(KammusuLuckTextBox.Text);
				if (luck < -1 || 100 < luck) {
					e.Cancel = true;
					error_provider_luck.SetError(KammusuLuckTextBox, "-1～100の値を入力してください");//range error
				}
				else {
					error_provider_luck.SetError(KammusuLuckTextBox, "");//error clear
				}
			}
			catch (Exception) {
				e.Cancel = true;
				error_provider_luck.SetError(KammusuLuckTextBox, "整数値以外の文字が入力されました！！");
			}
		}
		private void KammusuLuckTextBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
			)
				FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].luck = limit(int.Parse(KammusuLuckTextBox.Text), -1, 100);
		}
		private void KammusuCondTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int cond = int.Parse(KammusuCondTextBox.Text);
				if (cond < 0 || 100 < cond) {
					e.Cancel = true;
					error_provider_cond.SetError(KammusuCondTextBox, "0-100の値を入力してください");//range error
				}
				else {
					error_provider_cond.SetError(KammusuCondTextBox, "");//error clear
				}
			}
			catch (Exception) {
				e.Cancel = true;
				error_provider_cond.SetError(KammusuCondTextBox, "整数値以外の文字が入力されました！！");
			}
		}
		private void KammusuCondTextBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
			)
				FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].cond = limit(int.Parse(KammusuCondTextBox.Text), 0, 100);
		}
		private void ChangeKammusuButton_Click(object sender, EventArgs e) {
			if(KammusuTypeComboBox.SelectedIndex == -1
			|| KammusuNameComboBox.SelectedIndex == -1
			|| FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// 艦娘データを作成する
			int index = KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex];
			var id = data.Ships[index].艦船ID;
			var level = KammusuLevelTextBox.Text.ParseInt();
			var luck = KammusuLuckTextBox.Text.ParseInt();
			var cond = KammusuCondTextBox.Text.ParseInt();
			var setKammusu = new Kammusu(id, level, luck, cond);
			// 作成した艦娘データで上書きする
			FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex] = setKammusu;
			KammusuSelectListBox.Items[KammusuSelectListBox.SelectedIndex] = setKammusu.艦名;
			KammusuSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void DeleteKammusuButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから艦娘データを削除する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].RemoveAt(KammusuSelectListBox.SelectedIndex);
			KammusuSelectListBox.Items.RemoveAt(KammusuSelectListBox.SelectedIndex);
			KammusuSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void AddWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponTypeComboBox.SelectedIndex == -1
			|| WeaponNameComboBox.SelectedIndex == -1)
				return;
			// 4つ以上の装備は持てない
			Kammusu selectedKammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			if(selectedKammusu.items.Count == selectedKammusu.maxSlots)
				return;
			// 装備データを作成する
			int index = WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex][WeaponNameComboBox.SelectedIndex];
			var id = data.Weapons[index].装備ID;
			var level = WeaponLevelComboBox.SelectedIndex;
			var rf = WeaponRfComboBox.SelectedIndex;
			var detailRf = WeaponDetailRfComboBox.SelectedIndex;
			var setWeapon = new Weapon(id, level, rf, detailRf);
			// 作成した装備データを追加する
			selectedKammusu.items.Add(setWeapon);
			WeaponSelectListBox.Items.Add(setWeapon.装備名);
			WeaponSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void WeaponLevelComboBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
				&& IsVaidIndex(
					this.WeaponSelectListBox.SelectedIndex,
					this.FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].items.Count
				)
			)
				this
					.FormFleet
					.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex]
					.items[WeaponSelectListBox.SelectedIndex]
					.level = WeaponLevelComboBox.SelectedIndex;
		}
		private void WeaponRfComboBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
				&& IsVaidIndex(
					this.WeaponSelectListBox.SelectedIndex,
					this.FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].items.Count
				)
			) {
				this
					.FormFleet
					.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex]
					.items[WeaponSelectListBox.SelectedIndex]
					.rf = WeaponRfComboBox.SelectedIndex;
			}
		}
		private void WeaponDetailRfComboBox_Leave(object sender, EventArgs e) {
			if(//Range Check
				IsVaidIndex(this.FleetSelectComboBox.SelectedIndex, this.FormFleet.unit.Count)
				&& IsVaidIndex(this.KammusuSelectListBox.SelectedIndex, this.FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count)
				&& IsVaidIndex(
					this.WeaponSelectListBox.SelectedIndex,
					this.FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].items.Count
				)
			) {
				this
					.FormFleet
					.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex]
					.items[WeaponSelectListBox.SelectedIndex]
					.rf_detail = WeaponDetailRfComboBox.SelectedIndex;
			}
		}
		private void ChangeWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponTypeComboBox.SelectedIndex == -1
			|| WeaponNameComboBox.SelectedIndex == -1
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			Kammusu selectedKammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			// 装備データを作成する
			int index = WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex][WeaponNameComboBox.SelectedIndex];
			var id = data.Weapons[index].装備ID;
			var level = WeaponLevelComboBox.SelectedIndex.limit(0, 10);
			var rf = WeaponRfComboBox.SelectedIndex.limit(0, 7);
			var detailRf = WeaponDetailRfComboBox.SelectedIndex.limit(0, 120);
			var setWeapon = new Weapon(id, level, rf, detailRf);
			// 作成した装備データで上書きする
			selectedKammusu.items[WeaponSelectListBox.SelectedIndex] = setWeapon;
			WeaponSelectListBox.Items[WeaponSelectListBox.SelectedIndex] = setWeapon.装備名;
			WeaponSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void DeleteWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから装備データを削除する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].items.RemoveAt(WeaponSelectListBox.SelectedIndex);
			WeaponSelectListBox.Items.RemoveAt(WeaponSelectListBox.SelectedIndex);
			WeaponSelectListBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void HQLevelTextBox_TextChanged(object sender, EventArgs e) {
			// 司令部レベルが書き換わった際は反映する
			FormFleet.level = HQLevelTextBox.Text.ParseInt().limit(1, 120);
			RedrawSearchPower();
		}
		private void FleetTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetTypeComboBox.SelectedIndex == -1)
				return;
			// 艦隊形式が書き換わった際は反映する
			FormFleet.type = FleetTypeComboBox.SelectedIndex;
		}
		private void FleetSelectComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1)
				return;
			// 表示する艦隊を切り換える
			KammusuSelectListBox.ClearSelected();
			KammusuSelectListBox.Items.Clear();
			foreach(Kammusu kammusu in FormFleet.unit[FleetSelectComboBox.SelectedIndex]) {
				KammusuSelectListBox.Items.Add(data.Ships.Single(s => s.艦船ID == kammusu.id).艦名);
			}
			KammusuSelectListBox.Refresh();
		}
		private void KammusuSelectListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// 表示する艦娘を切り替える
			Kammusu kammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			var showKammusu = data.Ships.Single(s => s.艦船ID == kammusu.id);
			int showKammusuType = showKammusu.艦種 - 1;
			KammusuTypeComboBox.SelectedIndex = showKammusuType;
			KammusuTypeComboBox.Refresh();
			KammusuNameComboBox.Text = showKammusu.艦名;
			RedrawKammusuNameList();
			KammusuLevelTextBox.Text = kammusu.lv.ToString();
			KammusuLuckTextBox.Text = kammusu.luck.ToString();
			KammusuCondTextBox.Text = kammusu.cond.ToString();
			// 装備一覧を更新する
			WeaponSelectListBox.Items.Clear();
			foreach(var weapon in kammusu.items) {
				WeaponSelectListBox.Items.Add(data.Weapons.Single(w => w.装備ID == weapon.id).装備名);
			}
			WeaponSelectListBox.Refresh();
		}
		private void KammusuTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawKammusuNameList();
		}
		private void WeaponTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawWeaponNameList();
		}
		private void WeaponSelectListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			// 表示する装備を切り替える
			Kammusu kammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			Weapon weapon = kammusu.items[WeaponSelectListBox.SelectedIndex];
			var showWeapon = data.Weapons.Single(w => w.装備ID == weapon.id);
			int showWeaponType = WeaponTypeToNumber["その他"];
			if(WeaponTypeToNumber.ContainsKey(showWeapon.種別)) {
				showWeaponType = WeaponTypeToNumber[showWeapon.種別];
			}
			WeaponTypeComboBox.SelectedIndex = showWeaponType;
			WeaponTypeComboBox.Refresh();
			WeaponNameComboBox.Text = showWeapon.装備名;
			RedrawWeaponNameList();
			WeaponLevelComboBox.SelectedIndex = weapon.level;
			WeaponLevelComboBox.Refresh();
			WeaponRfComboBox.SelectedIndex = weapon.rf;
			WeaponRfComboBox.Refresh();
			WeaponDetailRfComboBox.SelectedIndex = weapon.rf_detail;
			WeaponDetailRfComboBox.Refresh();
		}
		private void WeaponRfComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			// 外部熟練度を弄った場合、内部熟練度を自動補正する
			WeaponDetailRfComboBox.SelectedIndex = Weapon.ToDetail(WeaponRfComboBox.SelectedIndex);
			WeaponDetailRfComboBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void WeaponDetailRfComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			// 内部熟練度を弄った場合、外部熟練度を自動補正する
			WeaponRfComboBox.SelectedIndex = Weapon.FromDetail(WeaponDetailRfComboBox.SelectedIndex);
			WeaponRfComboBox.Refresh();
			RedrawAntiAirScore();
			RedrawSearchPower();
		}
		private void MainForm_DragDrop(object sender, DragEventArgs e) {
			if(MainTabControl.SelectedIndex == 0) {
				// ドラッグされたファイルを認識する
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
				if(files.Length < 1)
					return;
				FleetFilePath = files[0];
				// 拡張子で判別する
				if(Path.GetExtension(FleetFilePath) != ".json")
					return;
				// ファイルを詠みこみ、JSONとして解釈する
				Tuple<Fleet, bool> setFleet = ReadJsonFile(FleetFilePath);
				if(!setFleet.Item2)
					return;
				// 読み込んだデータを画面に反映する
				FormFleet = setFleet.Item1;
				HQLevelTextBox.Text = FormFleet.level.ToString();
				FleetTypeComboBox.SelectedIndex = FormFleet.type;
				FleetSelectComboBox_SelectedIndexChanged(sender, e);
				RedrawAntiAirScore();
				RedrawSearchPower();
			} else if(MainTabControl.SelectedIndex == 1) {
				// ドラッグされたファイルを認識する
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
				if(files.Length < 1)
					return;
				MapFilePath = files[0];
				// 拡張子で判別する
				if(Path.GetExtension(MapFilePath) != ".map")
					return;
				// ファイルを詠みこみ、JSONとして解釈する
				Tuple<MapData, bool> setMapData = ReadMapFile(MapFilePath);
				if(!setMapData.Item2)
					return;
				// 読み込んだデータを画面に反映する
				FormMapData = setMapData.Item1;
				MapPositionListBox.Items.Clear();
				foreach(var position in FormMapData.position) {
					MapPositionListBox.Items.Add(position.name);
				}
				MapPositionListBox.Refresh();
				RedrawMapAntiAirScore();
			}
		}
		private void MainForm_DragEnter(object sender, DragEventArgs e) {
			if(e.Data.GetDataPresent(DataFormats.FileDrop)) {
				e.Effect = DragDropEffects.All;
			} else {
				e.Effect = DragDropEffects.None;
			}
		}

		// マップエディタタブ
		private void AddMapPositionButton_Click(object sender, EventArgs e) {
			if(MapPositionNameTextBox.Text == ""
			|| MapPositionBattleModeComboBox.SelectedIndex == -1)
				return;
			var setPosition = new Position();
			setPosition.name = MapPositionNameTextBox.Text;
			setPosition.mode = MapPositionBattleModeComboBox.SelectedIndex;
			FormMapData.position.Add(setPosition);
			MapPositionListBox.Items.Add(setPosition.name);
			MapPositionListBox.Refresh();
		}
		private void ChangeMapPositionButton_Click(object sender, EventArgs e) {
			if(MapPositionNameTextBox.Text == ""
			|| MapPositionBattleModeComboBox.SelectedIndex == -1
			|| MapPositionListBox.SelectedIndex == -1)
				return;
			FormMapData.position[MapPositionListBox.SelectedIndex].name = MapPositionNameTextBox.Text;
			FormMapData.position[MapPositionListBox.SelectedIndex].mode = MapPositionBattleModeComboBox.SelectedIndex;
			MapPositionListBox.Items[MapPositionListBox.SelectedIndex] = MapPositionNameTextBox.Text;
			MapPositionListBox.Refresh();
		}
		private void DeleteMapPositionButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1)
				return;
			FormMapData.position.RemoveAt(MapPositionListBox.SelectedIndex);
			MapPositionListBox.Items.RemoveAt(MapPositionListBox.SelectedIndex);
			MapPositionListBox.Refresh();
		}
		private void AddMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternFormationComboBox.SelectedIndex == -1)
				return;
			// 艦隊を追加
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			var setFleet = new Fleet();
			setFleet.level = 120;
			setFleet.type = 0;
			selectPosition.fleet.Add(setFleet);
			selectPosition.formation.Add(MapPatternFormationComboBox.SelectedIndex);
			// 画面上に反映
			int selectPositionCount = selectPosition.fleet.Count;
			MapPatternListBox.Items.Add(selectPositionCount.ToString() + " : " + selectPosition.fleet[selectPositionCount - 1].unit[0].Count.ToString() + "隻");
			MapPatternListBox.Refresh();
		}
		private void ChangeMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternFormationComboBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			FormMapData.position[MapPositionListBox.SelectedIndex].formation[MapPatternListBox.SelectedIndex] = MapPatternFormationComboBox.SelectedIndex;
		}
		private void DeleteMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			selectPosition.fleet.RemoveAt(MapPatternListBox.SelectedIndex);
			selectPosition.formation.RemoveAt(MapPatternListBox.SelectedIndex);
			MapPatternListBox.Items.Clear();
			for(int fi = 0; fi < selectPosition.fleet.Count; ++fi) {
				MapPatternListBox.Items.Add((fi + 1).ToString() + " : " + selectPosition.fleet[fi].unit[0].Count.ToString());
			}
			MapPatternListBox.Refresh();
		}
		private void AddMapKammusuButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuTypeComboBox.SelectedIndex == -1
			|| MapKammusuNameComboBox.SelectedIndex == -1)
				return;
			if(FormMapData.position[MapPositionListBox.SelectedIndex].fleet[MapPatternListBox.SelectedIndex].unit[0].Count >= MaxUnitSize)
				return;
			// 艦娘データを作成
			int index = KammusuTypeToIndexList[MapKammusuTypeComboBox.SelectedIndex][MapKammusuNameComboBox.SelectedIndex];
			var setKammusu = new Kammusu(data.Ships[index].艦船ID);
			// 艦娘データを追加
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			var selectFleet = selectPosition.fleet[MapPatternListBox.SelectedIndex];
			int selectPositionCount = selectPosition.fleet.Count;
			selectFleet.unit[0].Add(setKammusu);
			MapKammusuListBox.Items.Add(setKammusu.艦名);
			MapKammusuListBox.Refresh();
			MapPatternListBox.Items[MapPatternListBox.SelectedIndex] = selectPositionCount.ToString() + " : " + selectFleet.unit[0].Count.ToString() + "隻";
			RedrawMapAntiAirScore();
		}
		private void ChangeMapKammusuButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuTypeComboBox.SelectedIndex == -1
			|| MapKammusuNameComboBox.SelectedIndex == -1
			|| MapKammusuListBox.SelectedIndex == -1)
				return;
			// 艦娘データを作成
			int index = KammusuTypeToIndexList[MapKammusuTypeComboBox.SelectedIndex][MapKammusuNameComboBox.SelectedIndex];
			var setKammusu = new Kammusu(data.Ships[index].艦船ID);
			// 艦娘データを追加
			var selectFleet = FormMapData.position[MapPositionListBox.SelectedIndex].fleet[MapPatternListBox.SelectedIndex];
			selectFleet.unit[0][MapKammusuListBox.SelectedIndex] = setKammusu;
			MapKammusuListBox.Items[MapKammusuListBox.SelectedIndex] = data.Ships.Single(s => s.艦船ID == setKammusu.id).艦名;
			MapKammusuListBox.Refresh();
			RedrawMapAntiAirScore();
		}
		private void DeleteMapKammusuButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuListBox.SelectedIndex == -1)
				return;
			FormMapData.position[MapPositionListBox.SelectedIndex].fleet[MapPatternListBox.SelectedIndex].unit[0].RemoveAt(MapKammusuListBox.SelectedIndex);
			MapKammusuListBox.Items.RemoveAt(MapKammusuListBox.SelectedIndex);
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			var selectFleet = selectPosition.fleet[MapPatternListBox.SelectedIndex];
			int selectPositionCount = selectPosition.fleet.Count;
			MapPatternListBox.Items[MapPatternListBox.SelectedIndex] = selectPositionCount.ToString() + " : " + selectFleet.unit[0].Count.ToString() + "隻";
			RedrawMapAntiAirScore();
		}
		private void MapKammusuTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawMapKammusuNameList();
		}
		private void MapPositionListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1)
				return;
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			// 選択したマスについて、その名前と戦闘モードに関する情報
			MapPositionNameTextBox.Text = selectPosition.name;
			MapPositionBattleModeComboBox.SelectedIndex = selectPosition.mode;
			MapPositionBattleModeComboBox.Refresh();
			// 選択したマスについて、それに含まれるパターンに関する情報
			MapPatternListBox.Items.Clear();
			for(int pi = 0; pi < selectPosition.fleet.Count; ++pi) {
				MapPatternListBox.Items.Add((pi + 1).ToString() + " : " + selectPosition.fleet[pi].unit[0].Count.ToString() + "隻");
			}
			MapPatternListBox.Refresh();
		}
		private void MapPatternListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			// 選択したパターンについて、その陣形に関する情報
			MapPatternFormationComboBox.SelectedIndex = selectPosition.formation[MapPatternListBox.SelectedIndex];
			MapPatternFormationComboBox.Refresh();
			// 選択したパターンについて、それに含まれる艦娘に関する情報
			MapKammusuListBox.Items.Clear();
			var selectFleet = selectPosition.fleet[MapPatternListBox.SelectedIndex].unit[0];
			foreach(var kammusu in selectFleet) {
				MapKammusuListBox.Items.Add(data.Ships.Single(s => s.艦船ID == kammusu.id).艦名);
			}
			MapKammusuListBox.Refresh();
			RedrawMapAntiAirScore();
		}
		private void MapKammusuListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuListBox.SelectedIndex == -1)
				return;
			// 表示する艦娘を切り替える
			Kammusu kammusu = FormMapData.position[MapPositionListBox.SelectedIndex].fleet[MapPatternListBox.SelectedIndex].unit[0][MapKammusuListBox.SelectedIndex];
			var showKammusu = data.Ships.Single(s => s.艦船ID == kammusu.id);
			int showKammusuType = showKammusu.艦種 - 1;
			MapKammusuTypeComboBox.SelectedIndex = showKammusuType;
			MapKammusuTypeComboBox.Refresh();
			MapKammusuNameComboBox.Text = showKammusu.艦名;
			RedrawMapKammusuNameList();
		}

		// シミュレーションタブ
		private void FriendBrowseButton_Click(object sender, EventArgs e) {
			// ファイルを開くダイアログを表示する
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "艦隊データ(*.json)|*.json|すべてのファイル(*.*)|*.*";
			if(ofd.ShowDialog() != DialogResult.OK)
				return;
			FriendPathTextBox.Text = ofd.FileName;
		}
		private void EnemyBrowseButton_Click(object sender, EventArgs e) {
			// ファイルを開くダイアログを表示する
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "艦隊データ(*.json)|*.json|マップエディタ(*.map)|*.map|すべてのファイル(*.*)|*.*";
			if(ofd.ShowDialog() != DialogResult.OK)
				return;
			EnemyPathTextBox.Text = ofd.FileName;
		}
		private void OutputBrowseButton_Click(object sender, EventArgs e) {
			// ファイルを開くダイアログを表示する
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "艦隊データ(*.json)|*.json";
			if(sfd.ShowDialog() != DialogResult.OK)
				return;
			OutputPathTextBox.Text = sfd.FileName;
		}
		private void StartButton_Click(object sender, EventArgs e) {
			if(!System.IO.File.Exists(FriendPathTextBox.Text)
			|| !System.IO.File.Exists(EnemyPathTextBox.Text)
			|| (PutJSONCheckBox.Checked && OutputPathTextBox.Text == ""))
				return;
			ProcessStartInfo psInfo = new ProcessStartInfo();
			psInfo.FileName = @System.IO.Directory.GetCurrentDirectory() + @"\KCS_CUI.exe"; //実行するファイル
			if(!System.IO.File.Exists(psInfo.FileName)) {
				this.Text = SoftName;
				return;
			}
			string commandLine = "";
			commandLine += "-i \"" + FriendPathTextBox.Text + "\" \"" + EnemyPathTextBox.Text + "\"";
			commandLine += " -f " + FriendFormationComboBox.SelectedIndex.ToString() + " " + EnemyFormationComboBox.SelectedIndex.ToString();
			commandLine += " -n " + TimesComboBox.Text;
			commandLine += " -t " + ThreadsComboBox.Text;
			if(PutJSONCheckBox.Checked)
				commandLine += " -o \"" + OutputPathTextBox.Text + "\"";
			if(PrettifyJSONCheckBox.Checked) {
				commandLine += " --result-json-prettify";
			} else {
				commandLine += " --no-result-json-prettify";
			}
			//外部exeファイルで実行し、標準出力を取り込む
			this.Text = SoftName + "(計算中)";
			System.Console.WriteLine(commandLine);
			System.Console.WriteLine(psInfo.FileName);
			psInfo.CreateNoWindow = true;		   //コンソール・ウィンドウを開かない
			psInfo.UseShellExecute = false;		 //シェル機能を使用しない
			psInfo.Arguments = commandLine;		 //コマンドライン引数を設定
			psInfo.RedirectStandardOutput = true;   //標準出力を取り込むようにする
			Process p = Process.Start(psInfo);
			string output = p.StandardOutput.ReadToEnd();
			ResultTextBox.Text = output;
			ResultTextBox.Focus();
			this.Text = SoftName;
		}
		private void FriendPathTextBox_DragDrop(object sender, DragEventArgs e) {
			// ドラッグされたファイルを認識する
			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
			if(files.Length < 1)
				return;
			if(Path.GetExtension(files[0]) != ".json")
				return;
			FriendPathTextBox.Text = files[0];
		}
		private void FriendPathTextBox_DragEnter(object sender, DragEventArgs e) {
			if(e.Data.GetDataPresent(DataFormats.FileDrop)) {
				e.Effect = DragDropEffects.All;
			} else {
				e.Effect = DragDropEffects.None;
			}
		}
		private void EnemyPathTextBox_DragDrop(object sender, DragEventArgs e) {
			// ドラッグされたファイルを認識する
			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
			if(files.Length < 1)
				return;
			if(Path.GetExtension(files[0]) != ".json"
			&& Path.GetExtension(files[0]) != ".map")
				return;
			EnemyPathTextBox.Text = files[0];
		}
		private void EnemyPathTextBox_DragEnter(object sender, DragEventArgs e) {
			if(e.Data.GetDataPresent(DataFormats.FileDrop)) {
				e.Effect = DragDropEffects.All;
			} else {
				e.Effect = DragDropEffects.None;
			}
		}

		/* サブルーチン */
		// 装備データを読み込み
		private void ReadWeaponData() {
			// 読み込んだデータからインデックスを張る
			WeaponTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = data.Weapons.Select();
			for(int i = 0; i < dr.Length; ++i) {
				// 種類→インデックスは例外を考慮する
				int type = WeaponTypeToNumber["その他"];
				if(WeaponTypeToNumber.ContainsKey(dr[i]["種別"].ToString())) {
					type = WeaponTypeToNumber[dr[i]["種別"].ToString()];
				}
				if(!WeaponTypeToIndexList.ContainsKey(type)) {
					WeaponTypeToIndexList.Add(type, new List<int>());
				}
				WeaponTypeToIndexList[type].Add(i);
			}
		}
		// 艦娘データを読み込み
		private void ReadKammusuData() {
			// 読み込んだデータからインデックスを張る
			KammusuTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = data.Ships.Select();
			for(int i = 0; i < dr.Length; ++i) {
				// 種類→インデックスは例外を考慮する
				int type = dr[i]["艦種"].ToString().ParseInt() - 1;   //1を引くのはインデックスとの対応のため
				if(!KammusuTypeToIndexList.ContainsKey(type)) {
					KammusuTypeToIndexList.Add(type, new List<int>());
				}
				KammusuTypeToIndexList[type].Add(i);
			}
		}
		// 装備データをGUIに反映
		private void RedrawWeaponNameList() {
			WeaponNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(WeaponTypeComboBox.SelectedIndex < 0)
				return;
			foreach(int index in WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex]) {
				WeaponNameComboBox.Items.Add(data.Weapons[index].装備名);
			}
			WeaponNameComboBox.Refresh();
			// 改修度および熟練度の選択を切り替える
			if(RfWeaponTypeList.Contains(WeaponTypeComboBox.SelectedIndex)) {
				// 熟練度
				WeaponLevelComboBox.Enabled = false;
				WeaponRfComboBox.Enabled = true;
				WeaponDetailRfComboBox.Enabled = true;
			} else {
				// 改修度
				WeaponLevelComboBox.Enabled = true;
				WeaponRfComboBox.Enabled = false;
				WeaponDetailRfComboBox.Enabled = false;
			}
		}
		// 艦娘データをGUIに反映
		private void RedrawKammusuNameList() {
			KammusuNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(KammusuTypeComboBox.SelectedIndex < 0)
				return;
			foreach(int index in KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex]) {
				KammusuNameComboBox.Items.Add(data.Ships[index].艦名);
			}
			KammusuNameComboBox.Refresh();
		}
		private void RedrawMapKammusuNameList() {
			MapKammusuNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(MapKammusuTypeComboBox.SelectedIndex < 0)
				return;
			foreach(int index in KammusuTypeToIndexList[MapKammusuTypeComboBox.SelectedIndex]) {
				MapKammusuNameComboBox.Items.Add(data.Ships[index].艦名);
			}
			MapKammusuNameComboBox.Refresh();
		}
		// 値を上下限で制限する
		static public int limit(int n, int min_n, int max_n) {
			return (n < min_n) ? min_n : (max_n < n) ? max_n : n;
		}
		private Tuple<Fleet, bool> ReadJsonFile(string jsonFileName) {
			Fleet setFleet = new Fleet();
			// テキストを読み込んでJSONにパースする
			System.IO.StreamReader sr = new System.IO.StreamReader(jsonFileName, System.Text.Encoding.GetEncoding("utf-8"));
			string jsonString = sr.ReadToEnd();
			sr.Close();
			JObject json = JObject.Parse(jsonString);
			// 司令部レベルを読み込む
			if(json["lv"] != null) {
				setFleet.level = limit(int.Parse((string)json["lv"]), 1, 120);
			} else {
				setFleet.level = 120;
			}
			// 艦隊形式を読み込む
			if(json["type"] != null) {
				setFleet.type = limit(int.Parse((string)json["type"]), 0, 3);
			} else {
				setFleet.type = 0;
			}
			// 艦隊を読み込む
			for(int fi = 1; fi <= MaxFleetSize; ++fi) {
				if(json["f" + fi.ToString()] == null)
					break;
				var jsonFleet = (JObject)json["f" + fi.ToString()];
				// 艦娘を読み込む
				for(int si = 1; si <= MaxUnitSize; ++si) {
					// JSONデータとしての判定
					var jsonKammusu = jsonFleet["s" + si];
					if(jsonKammusu == null)
						break;
					try {
						setFleet.unit[fi - 1].Add(jsonKammusu.ToObject<Kammusu>());
					}
					catch {
						return new Tuple<Fleet, bool>(setFleet, false);
					}
				}
			}
			return new Tuple<Fleet, bool>(setFleet, true);
		}
		private Tuple<MapData, bool> ReadMapFile(string mapFileName) {
			var setMapData = new MapData();
			// テキストを読み込んでJSONにパースする
			System.IO.StreamReader sr = new System.IO.StreamReader(mapFileName, System.Text.Encoding.GetEncoding("utf-8"));
			string jsonString = sr.ReadToEnd();
			sr.Close();
			JObject json = JObject.Parse(jsonString);
			// 順番に読み込んでいく
			foreach(JObject jsonPosition in json["position"]) {
				// 各マス
				var position = new Position();
				position.name = (string)jsonPosition["name"];
				position.mode = int.Parse((string)jsonPosition["mode"]);
				foreach(JObject jsonPattern in jsonPosition["pattern"]) {
					var fleet = new Fleet();
					fleet.level = 120;
					fleet.type = 0;
					foreach(string jsonFleet in jsonPattern["fleets"]) {
						fleet.unit[0].Add(new Kammusu(int.Parse(jsonFleet)));
					}
					position.fleet.Add(fleet);
					position.formation.Add(int.Parse((string)jsonPattern["form"]));
				}
				setMapData.position.Add(position);
			}
			return new Tuple<MapData, bool>(setMapData, true);
		}
		// 制空値を計算して表示する(艦隊エディタ)
		private void RedrawAntiAirScore() {
			AllAntiAirTextBox.Text = FormFleet.CalcAntiAirScore().ToString();
		}
		// 索敵値を計算して表示する(艦隊エディタ)
		private void RedrawSearchPower() {
			double searchPower25A = FormFleet.CalcSearchPower25A();
			double searchPower33 = FormFleet.CalcSearchPower33();
			SearchPower25TextBox.Text = searchPower25A.ToString();
			SearchPower33TextBox.Text = searchPower33.ToString();
		}
		// 制空値を計算して表示する(マップエディタ)
		private void RedrawMapAntiAirScore() {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			int antiAirScore = FormMapData.position[MapPositionListBox.SelectedIndex].fleet[MapPatternListBox.SelectedIndex].CalcAntiAirScore();
			MapPatternAllAntiAirTextBox.Text = antiAirScore.ToString();
		}
		/* サブクラス */
		// 艦隊
		private class Fleet {
			// 司令部レベル
			public int level;
			// 艦隊形式
			public int type;
			// 艦娘
			public List<List<Kammusu>> unit;
			// コンストラクタ
			public Fleet() {
				level = 120;
				type = 0;
				unit = new List<List<Kammusu>>();
				for(int i = 0; i < MaxFleetSize; ++i) {
					unit.Add(new List<Kammusu>());
				}
			}
			// JSON書き出し
			public string ToJson() {
				JObject o = new JObject();
				o["version"] = 3;
				o["lv"] = level;
				o["type"] = type;
				// 書き出す艦隊数は艦隊形式によって制御する
				int writeFleets;
				if(type != 0) {
					writeFleets = 2;
				} else {
					writeFleets = 1;
				}
				// 艦隊を順に書き出していく
				for(int fi = 0; fi < writeFleets; ++fi) {
					// 艦隊
					JObject setFleet = new JObject();
					for(int si = 0; si < unit[fi].Count; ++si) {
						// 艦娘
						setFleet[$"s{si + 1}"] = JObject.FromObject(unit[fi][si]);
					}
					o["f" + (fi + 1).ToString()] = setFleet;
				}
				// 最後にデシリアライズしておしまい
				return o.ToString();
			}
			// 制空値計算
			public int CalcAntiAirScore() {
				int antiAirScore = 0;
				foreach(var kammusu in unit[0]) {
					var slots = data.Ships.Single(s => s.艦船ID == kammusu.id).搭載数.Split('/');
					for(int wi = 0; wi < kammusu.items.Count; ++wi) {
						var weapon = kammusu.items[wi];
						// まず装備の種類を読み取り、制空計算に使えるかを判別する
						var weaponInfo = data.Weapons.Single(w => w.装備ID == weapon.id);
						var type = weaponInfo.種別;
						if(type != "艦上戦闘機"
						&& type != "艦上爆撃機(爆戦)"
						&& type != "艦上爆撃機"
						&& type != "水上爆撃機"
						&& type != "艦上攻撃機"
						&& type != "水上戦闘機")
							continue;
						// 対空値・搭載数・内部熟練度で決まる制空値を代入する
						var antiAir = weaponInfo.対空;
						var slot = slots[wi].ParseInt();
						double antiAirScoreWeapon = antiAir * Math.Sqrt(slot) + Math.Sqrt(0.1 * weapon.rf_detail);
						// 一部の種別には特別な補正を掛ける
						if(type == "艦上戦闘機") {
							antiAirScoreWeapon += bonusPF[weapon.rf];
						} else if(type == "水上爆撃機") {
							antiAirScoreWeapon += bonusWB[weapon.rf];
						}
						// 小数点以下を切り捨てたものを加算する
						antiAirScore += (int)(antiAirScoreWeapon);
					}
				}
				return antiAirScore;
			}
			// 索敵値計算
			public double CalcSearchPower25A() {
				double searchPower = 0.0;
				// 司令部レベル補正
				int roundUp5Level = ((level - 1) / 5 + 1) * 5;
				searchPower += -0.6142467 * roundUp5Level;
				// 艦娘・装備による補正
				foreach(var kammusu in unit[0]) {
					// 艦娘の索敵値は練度依存
					var searchValueSet = data.Ships.Single(s => s.艦船ID == kammusu.id).索敵.Split('/');
					var searchValueK = searchValueSet[1].ParseInt();
					searchPower += Math.Sqrt(searchValueK) * 1.6841056;
					foreach(var weapon in kammusu.items) {
						// 装備の索敵値は種別によって係数が異なる
						var searchValueW = data.Weapons.Single(w => w.装備ID == weapon.id).索敵;
						switch(data.Weapons.Single(w => w.装備ID == weapon.id).種別) {
						case "艦上爆撃機":
							searchPower += 1.0376255 * searchValueW;
							break;
						case "艦上爆撃機(爆戦)":
							searchPower += 1.0376255 * searchValueW;
							break;
						case "水上爆撃機":
							searchPower += 1.7787282 * searchValueW;
							break;
						case "艦上攻撃機":
							searchPower += 1.3677954 * searchValueW;
							break;
						case "艦上偵察機":
							searchPower += 1.6592780 * searchValueW;
							break;
						case "艦上偵察機(彩雲)":
							searchPower += 1.6592780 * searchValueW;
							break;
						case "水上偵察機":
							searchPower += 2.0000000 * searchValueW;
							break;
						case "水上偵察機(夜偵)":
							searchPower += 2.0000000 * searchValueW;
							break;
						case "小型電探":
							searchPower += 1.0045358 * searchValueW;
							break;
						case "大型電探":
							searchPower += 0.9906638 * searchValueW;
							break;
						case "探照灯":
							searchPower += 0.9067950 * searchValueW;
							break;
						default:
							break;
						}
					}
				}
				// 小数第2位を四捨五入
				return 0.1 * Math.Round(10.0 * searchPower);
			}
			public double CalcSearchPower33() {
				double searchPower = 0.0;
				// 司令部レベル補正
				searchPower -= Math.Ceiling(0.4 * level);
				// 艦娘・装備による補正
				foreach(var kammusu in unit[0]) {
					// 艦娘の索敵値は練度依存
					var searchValueSet = data.Ships.Single(s => s.艦船ID == kammusu.id).索敵.Split('/');
					var searchValueK = searchValueSet[1].ParseInt();
					searchPower += Math.Sqrt(searchValueK);
					foreach(var weapon in kammusu.items) {
						// 装備の索敵値は種別によって係数が異なる
						var searchValueW = data.Weapons.Single(w => w.装備ID == weapon.id).索敵;
						switch(data.Weapons.Single(w => w.装備ID == weapon.id).種別) {
						case "水上爆撃機":
							searchPower += 1.1 * searchValueW;
							break;
						case "艦上攻撃機":
							searchPower += 0.8 * searchValueW;
							break;
						case "艦上偵察機":
							searchPower += 1.0 * searchValueW;
							break;
						case "艦上偵察機(彩雲)":
							searchPower += 1.0 * searchValueW;
							break;
						case "水上偵察機":
							searchPower += 1.2 * searchValueW + 1.2 * Math.Sqrt(weapon.level);
							break;
						case "水上偵察機(夜偵)":
							searchPower += 1.2 * searchValueW + 1.2 * Math.Sqrt(weapon.level);
							break;
						case "小型電探":
							searchPower += 0.6 * searchValueW + 1.25 * Math.Sqrt(weapon.level);
							break;
						case "大型電探":
							searchPower += 0.6 * searchValueW + 1.25 * Math.Sqrt(weapon.level);
							break;
						default:
							searchPower += 0.6 * searchValueW;
							break;
						}
					}
				}
				// 隻数による補正
				searchPower += 2.0 * (6.0 - unit[0].Count);
				// 小数第2位を四捨五入
				return 0.1 * Math.Round(10.0 * searchPower);
			}
		}
		// マスデータ
		private class Position {
			// マスにおける戦闘モード
			public int mode;
			// マスの名称
			public string name;
			// 各パターンにおける陣形
			public List<int> formation;
			// 各パターン
			public List<Fleet> fleet;
			// コンストラクタ
			public Position() {
				formation = new List<int>();
				fleet = new List<Fleet>();
			}
		}
		// マップデータ
		private class MapData {
			// マス
			public List<Position> position;
			// コンストラクタ
			public MapData() {
				position = new List<Position>();
			}
			// JSON書き出し
			public string ToJson() {
				JObject o = new JObject();
				o["version"] = "map";
				var jsonPositions = new JArray();
				// マップデータを書き出していく
				foreach(var pos in position) {
					var jsonPosition = new JObject();
					jsonPosition["name"] = pos.name;
					jsonPosition["mode"] = pos.mode;
					var jsonPatterns = new JArray();
					for(int pi = 0; pi < pos.fleet.Count; ++pi) {
						var jsonPattern = new JObject();
						jsonPattern["form"] = pos.formation[pi];
						var jsonFleets = new JArray();
						foreach(var kammusu in pos.fleet[pi].unit[0]) {
							jsonFleets.Add(kammusu.id);
						}
						jsonPattern["fleets"] = jsonFleets;
						jsonPatterns.Add(jsonPattern);
					}
					jsonPosition["pattern"] = jsonPatterns;
					jsonPositions.Add(jsonPosition);
				}
				o["position"] = jsonPositions;
				// 最後にデシリアライズしておしまい
				return o.ToString();
			}
		}
	}
	static class Extensions {
		public static int ParseInt(this string str) {
			return int.Parse(str);
		}
		public static int limit(this int n, int min, int max) {
			return MainForm.limit(n, min, max);
		}
	}
}
