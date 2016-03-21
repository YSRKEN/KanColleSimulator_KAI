using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace KCS_GUI
{
	public partial class MainForm : Form
	{
		/* メンバ変数 */
		// 定数群
		//ソフト名
		const string SoftName = "KanColleSimulator";
		//艦隊数の最大
		const int MaxFleetSize = 2;
		//艦隊における艦船数の最大
		const int MaxUnitSize = 6;
		//種別→種別番号変換
		static Dictionary<string, int> WeaponTypeToNumber;

		// 装備・艦娘データ
		//装備データ
		static DataTable WeaponData;
		//装備ID→インデックス変換
		static Dictionary<int, int> WeaponIDtoIndex;
		//種別番号→インデックスのリスト変換
		Dictionary<int, List<int>> WeaponTypeToIndexList;
		//熟練度が存在する装備の種別番号一覧
		static List<int> RfWeaponTypeList;
		//艦娘データ
		DataTable KammusuData;
		//艦船ID→インデックス
		Dictionary<int, int> KammusuIDtoIndex;
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

		/* コンストラクタ */
		public MainForm()
		{
			InitializeComponent();
			try
			{
				WeaponTypeToNumber = new Dictionary<string, int>() {
					{"主砲",0},
					{"対艦強化弾",1},
					{"副砲",2},
					{"魚雷",3},
					{"特殊潜航艇",4},
					{"艦上戦闘機",5},
					{"艦上爆撃機",6},
					{"艦上爆撃機(爆戦)",7},
					{"水上爆撃機",8},
					{"艦上攻撃機",9},
					{"艦上偵察機",10},
					{"艦上偵察機(彩雲)",11},
					{"大型飛行艇",12},
					{"水上偵察機",13},
					{"水上偵察機(夜偵)",14},
					{"対潜哨戒機",15},
					{"オートジャイロ",16},
					{"小型電探",17},
					{"大型電探",18},
					{"対空機銃",19},
					{"対空強化弾",20},
					{"高射装置",21},
					{"爆雷",22},
					{"ソナー",23},
					{"応急修理要員",24},
					{"探照灯",25},
					{"照明弾",26},
					{"艦隊司令部施設",27},
					{"水上艦要員",28},
					{"戦闘糧食",29},
					{"洋上補給",30},
					{"水上戦闘機",31},
					{"その他",32}
				};
				RfWeaponTypeList = new List<int> {
					WeaponTypeToNumber["艦上戦闘機"],
					WeaponTypeToNumber["艦上爆撃機"],
					WeaponTypeToNumber["艦上爆撃機(爆戦)"],
					WeaponTypeToNumber["水上爆撃機"],
					WeaponTypeToNumber["艦上攻撃機"],
					WeaponTypeToNumber["艦上偵察機"],
					WeaponTypeToNumber["艦上偵察機(彩雲)"],
					WeaponTypeToNumber["大型飛行艇"],
					WeaponTypeToNumber["水上戦闘機"]
				};
				ReadWeaponData();
				ReadKammusuData();
				RedrawWeaponNameList();
				RedrawKammusuNameList();
				RedrawMapKammusuNameList();
				FormFleet = new Fleet();
				FormMapData = new MapData();
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message, SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				this.Close();
			}
		}

		/* 各イベント毎の処理 */
		// メニュー
		private void NewFileMenuItem_Click(object sender, EventArgs e){
			FormFleet = new Fleet();
			HQLevelTextBox.Text = FormFleet.level.ToString();
			FleetTypeComboBox.SelectedIndex = FormFleet.type;
			FleetSelectComboBox_SelectedIndexChanged(sender, e);
		}
		private void OpenFileMenuItem_Click(object sender, EventArgs e){
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
			}
			return;
		}
		private void SaveSFileMenuItem_Click(object sender, EventArgs e){
			if(FleetFilePath == "") {
				SaveAFileMenuItem_Click(sender, e);
				return;
			}
			if(FormFleet.unit[0].Count == 0) {
				MessageBox.Show("艦娘を第1艦隊に1隻以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			// セーブデータを作成する
			string saveData = FormFleet.ToJson();
			// 作成したデータを保存する
			var sw = new StreamWriter(FleetFilePath, false, Encoding.GetEncoding("utf-8"));
			sw.Write(saveData);
			sw.Close();
		}
		private void SaveAFileMenuItem_Click(object sender, EventArgs e){
			if(FormFleet.unit[0].Count == 0) {
				MessageBox.Show("艦娘を第1艦隊に1隻以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			// セーブデータを作成する
			string saveData = FormFleet.ToJson();
			// 作成したデータを保存する
			var sfd = new SaveFileDialog();
			if(sfd.ShowDialog() != DialogResult.OK)
				return;
			FleetFilePath = sfd.FileName;
			var sw = new StreamWriter(FleetFilePath, false, Encoding.GetEncoding("utf-8"));
			sw.Write(saveData);
			sw.Close();
		}
		private void ExitMenuItem_Click(object sender, EventArgs e)
		{
			this.Close();
		}
		private void VersionInfoMenuItem_Click(object sender, EventArgs e){
			/* バージョン情報表示 */
			string verInfo = "";
			verInfo += SoftName + "\n";
			verInfo += "バージョン：1.5.0\n";
			verInfo += "作成者：YSR\n";
			verInfo += "最終更新日：2016/03/21";
			MessageBox.Show(verInfo, SoftName);
		}

		// 艦娘エディタタブ
		private void AddKammusuButton_Click(object sender, EventArgs e) {
			if(KammusuTypeComboBox.SelectedIndex == -1
			|| KammusuNameComboBox.SelectedIndex == -1
			|| FleetSelectComboBox.SelectedIndex == -1)
				return;
			DataRow[] dr = KammusuData.Select();
			// 1艦隊には6隻まで
			if(FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count == MaxUnitSize)
				return;
			// 艦娘データを作成する
			var setKammusu = new Kammusu();
			int index = KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex];
			setKammusu.id = int.Parse(dr[index]["艦船ID"].ToString());
			setKammusu.level = limit(int.Parse(KammusuLevelTextBox.Text), 1, 155);
			setKammusu.luck = int.Parse(KammusuLuckTextBox.Text);
			if(setKammusu.luck < 0) {
				// 運に負数を指定した場合は、艦娘のデフォルト値とする(デッキビルダーの仕様上、"-1"でOK)
				setKammusu.luck = -1;
			} else {
				// そうでない場合は、最大でも100までに抑える
				setKammusu.luck = limit(setKammusu.luck, 0, 100);
			}
			setKammusu.cond = limit(int.Parse(KammusuCondTextBox.Text), 0, 100);
			setKammusu.maxSlots = int.Parse(dr[index]["スロット数"].ToString());
			// 作成した艦娘データを追加する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].Add(setKammusu);
			KammusuSelectListBox.Items.Add(dr[KammusuIDtoIndex[setKammusu.id]]["艦名"].ToString());
			KammusuSelectListBox.Refresh();
		}
		private void ChangeKammusuButton_Click(object sender, EventArgs e) {
			if(KammusuTypeComboBox.SelectedIndex == -1
			|| KammusuNameComboBox.SelectedIndex == -1
			|| FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			DataRow[] dr = KammusuData.Select();
			// 艦娘データを作成する
			var setKammusu = new Kammusu();
			int index = KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex];
			setKammusu.id = int.Parse(dr[index]["艦船ID"].ToString());
			setKammusu.level = limit(int.Parse(KammusuLevelTextBox.Text), 1, 155);
			setKammusu.luck = int.Parse(KammusuLuckTextBox.Text);
			if(setKammusu.luck < 0) {
				// 運に負数を指定した場合は、艦娘のデフォルト値とする(デッキビルダーの仕様上、"-1"でOK)
				setKammusu.luck = -1;
			} else {
				// そうでない場合は、最大でも100までに抑える
				setKammusu.luck = limit(setKammusu.luck, 0, 100);
			}
			setKammusu.cond = limit(int.Parse(KammusuCondTextBox.Text), 0, 100);
			setKammusu.maxSlots = int.Parse(dr[index]["スロット数"].ToString());
			// 作成した艦娘データで上書きする
			FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex] = setKammusu;
			KammusuSelectListBox.Items[KammusuSelectListBox.SelectedIndex] = dr[KammusuIDtoIndex[setKammusu.id]]["艦名"].ToString();
			KammusuSelectListBox.Refresh();
		}
		private void DeleteKammusuButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから艦娘データを削除する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].RemoveAt(KammusuSelectListBox.SelectedIndex);
			KammusuSelectListBox.Items.RemoveAt(KammusuSelectListBox.SelectedIndex);
			KammusuSelectListBox.Refresh();
		}
		private void AddWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponTypeComboBox.SelectedIndex == -1
			|| WeaponNameComboBox.SelectedIndex == -1)
				return;
			// 4つ以上の装備は持てない
			Kammusu selectedKammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			if(selectedKammusu.weapon.Count == selectedKammusu.maxSlots)
				return;
			DataRow[] dr = WeaponData.Select();
			// 装備データを作成する
			var setWeapon = new Weapon();
			int index = WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex][WeaponNameComboBox.SelectedIndex];
			setWeapon.id = int.Parse(dr[index]["装備ID"].ToString());
			setWeapon.level = limit(WeaponLevelComboBox.SelectedIndex, 0, 10);
			setWeapon.rf = limit(WeaponRfComboBox.SelectedIndex, 0, 7);
			setWeapon.detailRf = limit(WeaponDetailRfComboBox.SelectedIndex, 0, 120);
			// 作成した装備データを追加する
			selectedKammusu.weapon.Add(setWeapon);
			WeaponSelectListBox.Items.Add(dr[WeaponIDtoIndex[setWeapon.id]]["装備名"].ToString());
			WeaponSelectListBox.Refresh();
		}
		private void ChangeWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponTypeComboBox.SelectedIndex == -1
			|| WeaponNameComboBox.SelectedIndex == -1
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			Kammusu selectedKammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			DataRow[] dr = WeaponData.Select();
			// 装備データを作成する
			var setWeapon = new Weapon();
			int index = WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex][WeaponNameComboBox.SelectedIndex];
			setWeapon.id = int.Parse(dr[index]["装備ID"].ToString());
			setWeapon.level = limit(WeaponLevelComboBox.SelectedIndex, 0, 10);
			setWeapon.rf = limit(WeaponRfComboBox.SelectedIndex, 0, 7);
			setWeapon.detailRf = limit(WeaponDetailRfComboBox.SelectedIndex, 0, 120);
			// 作成した装備データで上書きする
			selectedKammusu.weapon[WeaponSelectListBox.SelectedIndex] = setWeapon;
			WeaponSelectListBox.Items[WeaponSelectListBox.SelectedIndex] = dr[WeaponIDtoIndex[setWeapon.id]]["装備名"].ToString();
			WeaponSelectListBox.Refresh();
		}
		private void DeleteWeaponButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから装備データを削除する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex].weapon.RemoveAt(WeaponSelectListBox.SelectedIndex);
			WeaponSelectListBox.Items.RemoveAt(WeaponSelectListBox.SelectedIndex);
			WeaponSelectListBox.Refresh();
		}
		private void HQLevelTextBox_TextChanged(object sender, EventArgs e) {
			// 司令部レベルが書き換わった際は反映する
			FormFleet.level = limit(int.Parse(HQLevelTextBox.Text), 1, 120);
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
			DataRow[] dr = KammusuData.Select();
			foreach(Kammusu kammusu in FormFleet.unit[FleetSelectComboBox.SelectedIndex]) {
				KammusuSelectListBox.Items.Add(dr[KammusuIDtoIndex[kammusu.id]]["艦名"].ToString());
			}
			KammusuSelectListBox.Refresh();
		}
		private void KammusuSelectListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// 表示する艦娘を切り替える
			Kammusu kammusu = FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex];
			int showKammusuIndex = KammusuIDtoIndex[kammusu.id];
			DataRow[] dr = KammusuData.Select();
			int showKammusuType = int.Parse(dr[showKammusuIndex]["艦種"].ToString()) - 1;
			KammusuTypeComboBox.SelectedIndex = showKammusuType;
			KammusuTypeComboBox.Refresh();
			KammusuNameComboBox.Text = dr[showKammusuIndex]["艦名"].ToString();
			RedrawKammusuNameList();
			KammusuLevelTextBox.Text = kammusu.level.ToString();
			KammusuLuckTextBox.Text = kammusu.luck.ToString();
			KammusuCondTextBox.Text = kammusu.cond.ToString();
			// 装備一覧を更新する
			DataRow[] dr2 = WeaponData.Select();
			WeaponSelectListBox.Items.Clear();
			foreach(var weapon in kammusu.weapon) {
				WeaponSelectListBox.Items.Add(dr2[WeaponIDtoIndex[weapon.id]]["装備名"].ToString());
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
			Weapon weapon = kammusu.weapon[WeaponSelectListBox.SelectedIndex];
			int showWeaponIndex = WeaponIDtoIndex[weapon.id];
			DataRow[] dr = WeaponData.Select();
			int showWeaponType = WeaponTypeToNumber["その他"];
			if(WeaponTypeToNumber.ContainsKey(dr[showWeaponIndex]["種別"].ToString())) {
				showWeaponType = WeaponTypeToNumber[dr[showWeaponIndex]["種別"].ToString()];
			}
			WeaponTypeComboBox.SelectedIndex = showWeaponType;
			WeaponTypeComboBox.Refresh();
			WeaponNameComboBox.Text = dr[showWeaponIndex]["装備名"].ToString();
			RedrawWeaponNameList();
			WeaponLevelComboBox.SelectedIndex = weapon.level;
			WeaponLevelComboBox.Refresh();
			WeaponRfComboBox.SelectedIndex = weapon.rf;
			WeaponRfComboBox.Refresh();
			WeaponDetailRfComboBox.SelectedIndex = weapon.detailRf;
			WeaponDetailRfComboBox.Refresh();
		}
		private void WeaponRfComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			// 外部熟練度を弄った場合、内部熟練度を自動補正する
			WeaponDetailRfComboBox.SelectedIndex = rfRoughToDetail(WeaponRfComboBox.SelectedIndex);
			WeaponDetailRfComboBox.Refresh();
		}
		private void WeaponDetailRfComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			// 内部熟練度を弄った場合、外部熟練度を自動補正する
			WeaponRfComboBox.SelectedIndex = rfDetailToRough(WeaponDetailRfComboBox.SelectedIndex);
			WeaponRfComboBox.Refresh();
		}
		private void MainForm_DragDrop(object sender, DragEventArgs e) {
			// ドラッグされたファイルを認識する
			string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
			if(files.Length < 1)
				return;
			FleetFilePath = files[0];
			if(MainTabControl.SelectedIndex == 0) {
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

		}
		private void ChangeMapPatternButton_Click(object sender, EventArgs e) {

		}
		private void DeleteMapPatternButton_Click(object sender, EventArgs e) {

		}
		private void AddMapKammusuButton_Click(object sender, EventArgs e) {

		}
		private void ChangeMapKammusuButton_Click(object sender, EventArgs e) {

		}
		private void DeleteMapKammusuButton_Click(object sender, EventArgs e) {

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
				MapPatternListBox.Items.Add((pi + 1).ToString() + " : " + selectPosition.fleet[pi].unit[0].Count.ToString());
			}
			MapPatternListBox.Refresh();
		}
		private void MapPatternListBox_SelectedIndexChanged(object sender, EventArgs e) {
		}
		private void MapKammusuListBox_SelectedIndexChanged(object sender, EventArgs e) {
		}

		// シミュレーションタブ
		private void FriendBrowseButton_Click(object sender, EventArgs e){
			// ファイルを開くダイアログを表示する
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "艦隊データ(*.json)|*.json|すべてのファイル(*.*)|*.*";
			if(ofd.ShowDialog() != DialogResult.OK)
				return;
			FriendPathTextBox.Text = ofd.FileName;
		}
		private void EnemyBrowseButton_Click(object sender, EventArgs e){
			// ファイルを開くダイアログを表示する
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Filter = "艦隊データ(*.json)|*.json|マップエディタ(*.map)|*.map|すべてのファイル(*.*)|*.*";
			if(ofd.ShowDialog() != DialogResult.OK)
				return;
			EnemyPathTextBox.Text = ofd.FileName;
		}
		private void OutputBrowseButton_Click(object sender, EventArgs e){
			// ファイルを開くダイアログを表示する
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filter = "艦隊データ(*.json)|*.json";
			if(sfd.ShowDialog() != DialogResult.OK)
				return;
			OutputPathTextBox.Text = sfd.FileName;
		}
		private void StartButton_Click(object sender, EventArgs e){
			if(!System.IO.File.Exists(FriendPathTextBox.Text)
			|| !System.IO.File.Exists(EnemyPathTextBox.Text))
				return;
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
			ProcessStartInfo psInfo = new ProcessStartInfo();
			psInfo.FileName = @System.IO.Directory.GetCurrentDirectory() + @"\KCS_CUI.exe"; //実行するファイル
			System.Console.WriteLine(commandLine);
			System.Console.WriteLine(psInfo.FileName);
			psInfo.CreateNoWindow = true;           //コンソール・ウィンドウを開かない
			psInfo.UseShellExecute = false;         //シェル機能を使用しない
			psInfo.Arguments = commandLine;         //コマンドライン引数を設定
			psInfo.RedirectStandardOutput = true;   //標準出力を取り込むようにする
			Process p = Process.Start(psInfo);
			string output = p.StandardOutput.ReadToEnd();
			ResultTextBox.Text = output;
			ResultTextBox.Focus();
			this.Text = SoftName;
		}

		/* サブルーチン */
		// 装備データを読み込み
		private void ReadWeaponData()
		{
			// CSVをOLEで読み込む
			string csvDir = System.IO.Directory.GetCurrentDirectory();
			string csvFileName = "slotitems.csv";
			string conString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + csvDir + ";Extended Properties=\"text;HDR=Yes;FMT=Delimited\"";
			System.Data.OleDb.OleDbConnection con = new System.Data.OleDb.OleDbConnection(conString);
			string commText = "SELECT * FROM [" + csvFileName + "]";
			System.Data.OleDb.OleDbDataAdapter da = new System.Data.OleDb.OleDbDataAdapter(commText, con);
			WeaponData = new DataTable();
			da.Fill(WeaponData);
			// 読み込んだデータからインデックスを張る
			WeaponIDtoIndex = new Dictionary<int, int>();
			WeaponTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = WeaponData.Select();
			for (int i = 0; i < dr.Length; ++i){
				// ID→インデックスはそのまま
				WeaponIDtoIndex[int.Parse(dr[i]["装備ID"].ToString())] = i;
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
			string csvDir = System.IO.Directory.GetCurrentDirectory();
			string csvFileName = "ships.csv";
			string conString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + csvDir + ";Extended Properties=\"text;HDR=Yes;FMT=Delimited\"";
			System.Data.OleDb.OleDbConnection con = new System.Data.OleDb.OleDbConnection(conString);
			string commText = "SELECT * FROM [" + csvFileName + "]";
			System.Data.OleDb.OleDbDataAdapter da = new System.Data.OleDb.OleDbDataAdapter(commText, con);
			KammusuData = new DataTable();
			da.Fill(KammusuData);
			// 読み込んだデータからインデックスを張る
			KammusuIDtoIndex = new Dictionary<int, int>();
			KammusuTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = KammusuData.Select();
			for(int i = 0; i < dr.Length; ++i) {
				// ID→インデックスはそのまま
				KammusuIDtoIndex[int.Parse(dr[i]["艦船ID"].ToString())] = i;
				// 種類→インデックスは例外を考慮する
				int type = int.Parse(dr[i]["艦種"].ToString()) - 1;	//1を引くのはインデックスとの対応のため
				if(!KammusuTypeToIndexList.ContainsKey(type)) {
					KammusuTypeToIndexList.Add(type, new List<int>());
				}
				KammusuTypeToIndexList[type].Add(i);
			}
		}
		// 装備データをGUIに反映
		private void RedrawWeaponNameList(){
			WeaponNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(WeaponTypeComboBox.SelectedIndex < 0) return;
			DataRow[] dr = WeaponData.Select();
			foreach(int index in WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex]){
				WeaponNameComboBox.Items.Add(dr[index]["装備名"].ToString());
			}
			WeaponNameComboBox.Refresh();
			// 改修度および熟練度の選択を切り替える
			if(RfWeaponTypeList.IndexOf(WeaponTypeComboBox.SelectedIndex) != -1) {
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
			DataRow[] dr = KammusuData.Select();
			foreach(int index in KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex]) {
				KammusuNameComboBox.Items.Add(dr[index]["艦名"].ToString());
			}
			KammusuNameComboBox.Refresh();
		}
		private void RedrawMapKammusuNameList() {
			MapKammusuNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(MapKammusuTypeComboBox.SelectedIndex < 0)
				return;
			DataRow[] dr = KammusuData.Select();
			foreach(int index in KammusuTypeToIndexList[MapKammusuTypeComboBox.SelectedIndex]) {
				MapKammusuNameComboBox.Items.Add(dr[index]["艦名"].ToString());
			}
			MapKammusuNameComboBox.Refresh();
		}
		// 値を上下限で制限する
		private int limit(int n, int min_n, int max_n) {
			if(n < min_n)
				return min_n;
			if(n > max_n)
				return max_n;
			return n;
		}
		// 外部熟練度を内部熟練度に変換する
		private int rfRoughToDetail(int rf) {
			int[] roughToDetailList = new int[8] { 0, 10, 25, 40, 55, 70, 85, 100 };
			return roughToDetailList[rf];
		}
		// 内部熟練度を外部熟練度に変換する
		private int rfDetailToRough(int detailRf) {
			int roughRf;
			if(detailRf < 10)
				roughRf = 0;
			else if(detailRf < 25)
				roughRf = 1;
			else if(detailRf < 40)
				roughRf = 2;
			else if(detailRf < 55)
				roughRf = 3;
			else if(detailRf < 70)
				roughRf = 4;
			else if(detailRf < 85)
				roughRf = 5;
			else if(detailRf < 100)
				roughRf = 6;
			else
				roughRf = 7;
			return roughRf;
		}
		private Tuple<Fleet, bool> ReadJsonFile(string jsonFileName) {
			Fleet setFleet = new Fleet();
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
			DataRow[] drWeapon = WeaponData.Select();
			DataRow[] drKammusu = KammusuData.Select();
			for(int fi = 1; fi <= MaxFleetSize; ++fi) {
				if(json["f" + fi.ToString()] == null)
					break;
				var jsonFleet = (JObject)json["f" + fi.ToString()];
				// 艦娘を読み込む
				for(int si = 1; si <= MaxUnitSize; ++si) {
					// JSONデータとしての判定
					if(jsonFleet["s" + si.ToString()] == null)
						break;
					var jsonKammusu = (JObject)jsonFleet["s" + si.ToString()];
					if(jsonKammusu["id"] == null
					|| jsonKammusu["lv"] == null
					|| jsonKammusu["luck"] == null
					|| jsonKammusu["items"] == null)
						return new Tuple<Fleet, bool>(setFleet, false);
					var setKammusu = new Kammusu();
					// IDがデータベースに存在するか判定
					setKammusu.id = limit(int.Parse((string)jsonKammusu["id"]), 1, 999);
					if(!KammusuIDtoIndex.ContainsKey(setKammusu.id))
						return new Tuple<Fleet, bool>(setFleet, false);
					// 追記
					setKammusu.level = limit(int.Parse((string)jsonKammusu["lv"]), 1, 155);
					setKammusu.luck = limit(int.Parse((string)jsonKammusu["luck"]), -1, 100);
					var jsonItems = (JObject)jsonKammusu["items"];
					var slotSize = int.Parse(drKammusu[KammusuIDtoIndex[setKammusu.id]]["スロット数"].ToString());
					// 装備を読み込む
					for(int wi = 1; wi <= slotSize; ++wi) {
						// JSONデータとしての判定
						if(jsonItems["i" + wi.ToString()] == null)
							break;
						var jsonWeapon = (JObject)jsonItems["i" + wi.ToString()];
						if(jsonWeapon["id"] == null
						|| jsonWeapon["rf"] == null)
							return new Tuple<Fleet, bool>(setFleet, false);
						var setWeapon = new Weapon();
						// IDがデータベースに存在するか判定
						setWeapon.id = limit(int.Parse((string)jsonWeapon["id"]), 1, 999);
						if(!WeaponIDtoIndex.ContainsKey(setWeapon.id))
							return new Tuple<Fleet, bool>(setFleet, false);
						// 種別を判定することで、"rf"が装備改修度か艦載機熟練度かを判別する
						int setWeaponType = WeaponTypeToNumber["その他"];
						if(WeaponTypeToNumber.ContainsKey(drWeapon[WeaponIDtoIndex[setWeapon.id]]["種別"].ToString())) {
							setWeaponType = WeaponTypeToNumber[drWeapon[WeaponIDtoIndex[setWeapon.id]]["種別"].ToString()];
						}
						if(RfWeaponTypeList.IndexOf(setWeaponType) != -1) {
							setWeapon.level = 0;
							setWeapon.rf = limit(int.Parse((string)jsonWeapon["rf"]), 0, 7);
							if(jsonWeapon["rf_detail"] == null) {
								setWeapon.detailRf = rfRoughToDetail(setWeapon.rf);
							} else {
								setWeapon.detailRf = limit(int.Parse((string)jsonWeapon["rf_detail"]), 0, 120);
								setWeapon.rf = rfDetailToRough(setWeapon.detailRf);
							}
						} else {
							setWeapon.level = limit(int.Parse((string)jsonWeapon["rf"]), 0, 10);
							setWeapon.rf = 0;
							setWeapon.detailRf = 0;
						}
						setKammusu.weapon.Add(setWeapon);
					}
					setFleet.unit[fi - 1].Add(setKammusu);
				}
			}
			return new Tuple<Fleet, bool>(setFleet, true);
		}
		/* サブクラス */
		// 装備
		private class Weapon {
			// 装備ID
			public int id;
			// 装備改修度
			public int level;
			// 外部熟練度
			public int rf;
			// 内部熟練度
			public int detailRf;
		}
		// 艦娘
		private class Kammusu {
			// 艦船ID
			public int id;
			// レベル
			public int level;
			// 運
			public int luck;
			// cond値
			public int cond;
			// 装備
			public List<Weapon> weapon;
			// 最大スロット数
			public int maxSlots;
			// コンストラクタ
			public Kammusu() {
				weapon = new List<Weapon>();
			}
		}
		// 艦隊
		private class Fleet{
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
				}else {
					writeFleets = 1;
				}
				// 艦隊を順に書き出していく
				DataRow[] drWeapon = WeaponData.Select();
				for(int fi = 0; fi < writeFleets; ++fi) {
					// 艦隊
					JObject setFleet = new JObject();
					for(int si = 0; si < unit[fi].Count; ++si) {
						// 艦娘
						JObject setKammusu = new JObject();
						setKammusu["id"] = unit[fi][si].id;
						setKammusu["lv"] = unit[fi][si].level;
						setKammusu["luck"] = unit[fi][si].luck;
						setKammusu["cond"] = unit[fi][si].cond;
						//装備一覧
						JObject setItems = new JObject();
						for(int wi = 0; wi < unit[fi][si].weapon.Count; ++wi) {
							// 装備
							JObject setWeapon = new JObject();
							setWeapon["id"] = unit[fi][si].weapon[wi].id;
							//種別を判定することで、装備改修度か艦載機熟練度かを判別する
							int setWeaponType = WeaponTypeToNumber["その他"];
							if(WeaponTypeToNumber.ContainsKey(drWeapon[WeaponIDtoIndex[unit[fi][si].weapon[wi].id]]["種別"].ToString())) {
								setWeaponType = WeaponTypeToNumber[drWeapon[WeaponIDtoIndex[unit[fi][si].weapon[wi].id]]["種別"].ToString()];
							}
							if(RfWeaponTypeList.IndexOf(setWeaponType) != -1) {
								setWeapon["rf"] = unit[fi][si].weapon[wi].rf;
								setWeapon["detail_rf"] = unit[fi][si].weapon[wi].detailRf;
							}else {
								setWeapon["rf"] = unit[fi][si].weapon[wi].level;
							}
							setItems["i" + (wi + 1).ToString()] = setWeapon;
						}
						setKammusu["items"] = setItems;
						setFleet["s" + (si + 1).ToString()] = setKammusu;
					}
					o["f" + (fi + 1).ToString()] = setFleet;
				}
				// 最後にデシリアライズしておしまい
				return o.ToString();
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
		}
	}
}
