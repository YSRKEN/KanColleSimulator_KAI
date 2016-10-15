using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using static KCS_GUI.CsvDataSet;

namespace KCS_GUI {
	public partial class MainForm : Form {
		/* メンバ変数 */
		// 定数群
		//ソフト名
		const string SoftName = "KanColleSimulator";
		//艦隊における艦船数の最大
		const int MaxUnitSize = 6;
		//制空計算用(艦戦ボーナスと艦爆ボーナス)
		static List<double> bonusPF = new List<double> { 0.0, 0.0, 2.0, 5.0, 9.0, 14.0, 14.0, 22.0 };
		static List<double> bonusWB = new List<double> { 0.0, 0.0, 1.0, 1.0, 1.0, 3.0, 3.0, 6.0 };

		// 画面表示用データ
		//艦隊タブ
		Fleet FormFleet;
		//マップタブ
		MapData FormMapData;
		//艦隊タブにおけるファイルパス
		string FleetFilePath;
		//マップタブにおけるファイルパス
		string MapFilePath;
		//ファイルの状態
		OpenFileInfo[] file;
		//艦娘エディタ画面で艦娘が変更された時
		//ErrorProvider
		private System.Windows.Forms.ErrorProvider error_provider_level;
		private System.Windows.Forms.ErrorProvider error_provider_luck;
		private System.Windows.Forms.ErrorProvider error_provider_cond;
		/* コンストラクタ */
		public MainForm() {
			InitializeComponent();
			if(!System.IO.File.Exists(@System.IO.Directory.GetCurrentDirectory() + @"\Newtonsoft.Json.dll")) {
				this.Close();
			}
			try {
				KammusuTypeComboBox.DataSource = MapKammusuTypeComboBox.DataSource = data.Ships
					.Where(s => s.艦種 < shipTypes.Length)
					.OrderBy(s => s.艦種)
					.GroupBy(s => s.艦種, (t, g) => new { Key = shipTypes[t - 1], Value = g.ToArray() })
					.ToArray();
				WeaponTypeComboBox.DataSource = data.Weapons
					.GroupBy(w => WeaponTypeToNumber.ContainsKey(w.種別) ? w.種別 : "その他", (t, g) => new { Key = t, Value = g.ToArray() })
					.OrderBy(a => WeaponTypeToNumber[a.Key])
					.ToArray();
				RedrawWeaponNameList();
				RedrawKammusuNameList();
				RedrawMapKammusuNameList();
				FormFleet = new Fleet();
				FormMapData = new MapData();
				file = new OpenFileInfo[2]
				{
					new OpenFileInfo("untitled.json", FileState.new_created),
					new OpenFileInfo("untitled.map", FileState.new_created)
				};
				if (IsInRange(MainTabControl.SelectedIndex, 0, 1)) {
					OpenFileInfoToStatusBar(this.file[MainTabControl.SelectedIndex]);
				}
				error_provider_level = new System.Windows.Forms.ErrorProvider();
				error_provider_luck = new System.Windows.Forms.ErrorProvider();
				error_provider_cond = new System.Windows.Forms.ErrorProvider();
			} catch(Exception ex) {
				MessageBox.Show(ex.Message, SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				this.Close();
			}
		}

		/* 各イベント毎の処理 */
		private void OpenFileInfoToStatusBar(OpenFileInfo info) {
			this.filename_echo.ForeColor = (FileState.none != info.state) ? SystemColors.ControlText : SystemColors.GradientInactiveCaption;
			this.filename_echo.BackColor = info.bg_color;
			this.filename_echo.Text = info.name;
		}
		private void file_state_modified(FileState new_state) {
			if (IsInRange(MainTabControl.SelectedIndex, 0, 1)) {
				this.file[MainTabControl.SelectedIndex].UpdateState(new_state);
				OpenFileInfoToStatusBar(this.file[MainTabControl.SelectedIndex]);
			}
		}
		private void file_state_modified(string filename, FileState new_state) {
			if (IsInRange(MainTabControl.SelectedIndex, 0, 1)) {
				file_state_modified(new_state);
				if (FileState.none != new_state) {
					this.file[MainTabControl.SelectedIndex].name = this.filename_echo.Text = (0 == filename.Length) ? "untitled" : filename;
				}
			}
		}

		private void MainTAbControl_SelectedIndexChanged(object sender, EventArgs e) {
			if (MainTabControl.SelectedIndex < 0 || 1 < MainTabControl.SelectedIndex) {
				this.filename_echo.Text = "";
				this.filename_echo.BackColor = SystemColors.Control;
				this.filename_echo.ForeColor = SystemColors.ControlText;
			}
			else {
				this.filename_echo.Text = this.file[MainTabControl.SelectedIndex].name;
				this.filename_echo.BackColor = this.file[MainTabControl.SelectedIndex].bg_color;
				this.filename_echo.ForeColor = (FileState.none != this.file[MainTabControl.SelectedIndex].state) ? SystemColors.ControlText : SystemColors.GradientInactiveCaption;
			}
		}
		// メニュー
		private bool SaveChangeBreforeNewCreate() {
			return (IsInRange(MainTabControl.SelectedIndex, 0, 1))
				&& (FileState.modified != this.file[MainTabControl.SelectedIndex].state
				|| SaveChangeBeforeClear(MainTabControl.SelectedIndex));
		}
		private void NewFileMenuItem_Click(object sender, EventArgs e) {
			if (SaveChangeBreforeNewCreate()) {
				if (MainTabControl.SelectedIndex == 0) {
					FormFleet = new Fleet();
					HQLevelTextBox.Text = FormFleet.lv.ToString();
					FleetTypeComboBox.SelectedIndex = FormFleet.type - 1;
					FleetSelectComboBox_SelectedIndexChanged(sender, e);
					RedrawAntiAirScore();
					RedrawSearchPower();
					file_state_modified("untitled.json", FileState.new_created);
				} else if(MainTabControl.SelectedIndex == 1) {
					FormMapData = new MapData();
					MapPositionListBox.Items.Clear();
					MapPositionListBox.Refresh();
					RedrawMapAntiAirScore();
					file_state_modified("untitled.map", FileState.new_created);
				}
			}
		}
		private string filepath_to_name(string path) {
			return path.Substring(path.LastIndexOf('\\') + 1);
		}
		private void OpenFileMenuItem_Click(object sender, EventArgs e) {
			if (SaveChangeBreforeNewCreate()) {
				if (MainTabControl.SelectedIndex == 0) {
					// ファイルを開くダイアログを表示する
					OpenFileDialog ofd = new OpenFileDialog();
					ofd.Filter = "艦隊データ(*.json)|*.json|すべてのファイル(*.*)|*.*";
					if (ofd.ShowDialog() != DialogResult.OK)
						return;
					FleetFilePath = ofd.FileName;
					// ファイルを詠みこみ、JSONとして解釈する
					FormFleet = Fleet.ReadFrom(FleetFilePath);
					if(FormFleet == null)
						return;
					// 読み込んだデータを画面に反映する
					file_state_modified(filepath_to_name(this.FleetFilePath), FileState.saved);
					HQLevelTextBox.Text = FormFleet.lv.ToString();
					FleetTypeComboBox.SelectedIndex = FormFleet.type - 1;
					FleetSelectComboBox_SelectedIndexChanged(sender, e);
					RedrawAntiAirScore();
					RedrawSearchPower();
				}
				else if (MainTabControl.SelectedIndex == 1) {
					// ファイルを開くダイアログを表示する
					OpenFileDialog ofd = new OpenFileDialog();
					ofd.Filter = "マップデータ(*.map)|*.map|すべてのファイル(*.*)|*.*";
					if (ofd.ShowDialog() != DialogResult.OK)
						return;
					MapFilePath = ofd.FileName;
					// ファイルを詠みこみ、マップデータのJSONとして解釈する
					FormMapData = MapData.ReadFrom(MapFilePath);
					if (FormMapData == null)
						return;
					// 読み込んだデータを画面に反映する
					file_state_modified(filepath_to_name(this.MapFilePath), FileState.saved);
					MapPositionListBox.Items.Clear();
					foreach (var position in FormMapData.position) {
						MapPositionListBox.Items.Add(position.name);
					}
					MapPositionListBox.Refresh();
					RedrawMapAntiAirScore();
				}
			}
		}
		private bool CreateFleetFile() {
			var sfd = new SaveFileDialog();
			sfd.ShowHelp = true;//http://stackoverflow.com/questions/17163784/default-name-with-openfiledialog-c
			sfd.FileName = this.file[0].name;
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
			// 作成したデータを保存する
			FormFleet.WriteTo(FleetFilePath);
			file_state_modified(filepath_to_name(this.FleetFilePath), FileState.saved);
		}
		private bool CreateMapFile() {
			var sfd = new SaveFileDialog();
			sfd.ShowHelp = true;//http://stackoverflow.com/questions/17163784/default-name-with-openfiledialog-c
			sfd.FileName = this.file[1].name;
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
			if (FormMapData.position[0].pattern.Count == 0) {
				MessageBox.Show("艦隊をパターンに1つ以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if (FormMapData.position[0].pattern[0].fleets.Count == 0) {
				MessageBox.Show("艦娘を艦隊に1隻以上登録してください.", SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			if ((MapFilePath == null || MapFilePath == "" || force_create) && !CreateMapFile()) return;
			// 作成したデータを保存する
			FormMapData.WriteTo(MapFilePath);
			file_state_modified(filepath_to_name(this.MapFilePath), FileState.saved);
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
			this.Close();//jump to MainForm_FormClosing
		}
		// ヘルプの表示
		private void ShowHelpToolStripMenuItem_Click(object sender, EventArgs e) {
			System.Diagnostics.Process.Start("https://YSRKEN.github.io/KanColleSimulator_KAI/gui/index.html");
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
			var kammusu = (CsvDataSet.ShipsRow)KammusuNameComboBox.SelectedItem;
			if (kammusu == null || FleetSelectComboBox.SelectedIndex == -1)
				return;
			// 1艦隊には6隻まで
			if(FormFleet.unit[FleetSelectComboBox.SelectedIndex].Count == MaxUnitSize)
				return;
			// 艦娘データを作成する
			var id = kammusu.艦船ID;
			var level = KammusuLevelTextBox.Text.ParseInt();
			var luck = KammusuLuckTextBox.Text.ParseInt();
			var cond = KammusuCondTextBox.Text.ParseInt();
			var setKammusu = new Kammusu(id, level, luck, cond);
			// 作成した艦娘データを追加する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].Add(setKammusu);
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		static private bool IsInRange(int val, int min, int max) {
			return (min <= val && val <= max);
		}
		static private bool IsVaidIndex(int val, int size) {
			return IsInRange(val, 0, size - 1);
		}
		private void KammusuLevelTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int level = KammusuLevelTextBox.Text.ParseInt();
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
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			if (kammusu != null)
				kammusu.lv = KammusuLevelTextBox.Text.ParseInt();
		}
		private void KammusuLuckTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int luck = KammusuLuckTextBox.Text.ParseInt();
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
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			if (kammusu != null) {
				kammusu.luck = KammusuLuckTextBox.Text.ParseInt();
				file_state_modified(FileState.modified);
			}
		}
		private void KammusuCondTextBox_Validating(object sender, CancelEventArgs e) {
			try {
				int cond = KammusuCondTextBox.Text.ParseInt();
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
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			if (kammusu != null) {
				kammusu.cond = KammusuCondTextBox.Text.ParseInt();
				file_state_modified(FileState.modified);
			}
		}
		private void ChangeKammusuButton_Click(object sender, EventArgs e) {
			var kammusu = (CsvDataSet.ShipsRow)KammusuNameComboBox.SelectedItem;
			if(kammusu == null
			|| FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// 艦娘データを作成する
			var id = kammusu.艦船ID;
			var level = KammusuLevelTextBox.Text.ParseInt();
			var luck = KammusuLuckTextBox.Text.ParseInt();
			var cond = KammusuCondTextBox.Text.ParseInt();
			var setKammusu = new Kammusu(id, level, luck, cond);
			// 作成した艦娘データで上書きする
			FormFleet.unit[FleetSelectComboBox.SelectedIndex][KammusuSelectListBox.SelectedIndex] = setKammusu;
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void DeleteKammusuButton_Click(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1
			|| KammusuSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから艦娘データを削除する
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].RemoveAt(KammusuSelectListBox.SelectedIndex);
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void AddWeaponButton_Click(object sender, EventArgs e) {
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			var weapon = (CsvDataSet.WeaponsRow)WeaponNameComboBox.SelectedItem;
			if (kammusu == null || weapon == null)
				return;
			// 4つ以上の装備は持てない
			if(kammusu.items.Count == kammusu.maxSlots)
				return;
			// 装備データを作成する
			var id = weapon.装備ID;
			var level = WeaponLevelComboBox.SelectedIndex;
			var rf = WeaponRfComboBox.SelectedIndex;
			var detailRf = WeaponDetailRfComboBox.SelectedIndex;
			var setWeapon = new Weapon(id, level, rf, detailRf);
			// 作成した装備データを追加する
			kammusu.items.Add(setWeapon);
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void WeaponLevelComboBox_Leave(object sender, EventArgs e) {
			var weapon = (Weapon)WeaponSelectListBox.SelectedItem;
			if (weapon != null) {
				weapon.level = WeaponLevelComboBox.SelectedIndex;
				file_state_modified(FileState.modified);
			}
		}
		private void WeaponRfComboBox_Leave(object sender, EventArgs e) {
			var weapon = (Weapon)WeaponSelectListBox.SelectedItem;
			if (weapon != null) {
				weapon.rf = WeaponRfComboBox.SelectedIndex;
				file_state_modified(FileState.modified);
			}
		}
		private void WeaponDetailRfComboBox_Leave(object sender, EventArgs e) {
			var weapon = (Weapon)WeaponSelectListBox.SelectedItem;
			if (weapon != null) {
				weapon.rf_detail = WeaponDetailRfComboBox.SelectedIndex;
				file_state_modified(FileState.modified);
			}
		}
		private void ChangeWeaponButton_Click(object sender, EventArgs e) {
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			var weapon = (CsvDataSet.WeaponsRow)WeaponNameComboBox.SelectedItem;
			if(kammusu == null || weapon==null
			|| WeaponSelectListBox.SelectedIndex == -1)
				return;
			// 装備データを作成する
			var id = weapon.装備ID;
			var level = WeaponLevelComboBox.SelectedIndex.limit(0, 10);
			var rf = WeaponRfComboBox.SelectedIndex.limit(0, 7);
			var detailRf = WeaponDetailRfComboBox.SelectedIndex.limit(0, 120);
			var setWeapon = new Weapon(id, level, rf, detailRf);
			// 作成した装備データで上書きする
			kammusu.items[WeaponSelectListBox.SelectedIndex] = setWeapon;
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void DeleteWeaponButton_Click(object sender, EventArgs e) {
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			if (kammusu == null || WeaponSelectListBox.SelectedIndex == -1)
				return;
			// FormFleetから装備データを削除する
			kammusu.items.RemoveAt(WeaponSelectListBox.SelectedIndex);
			RedrawAntiAirScore();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void HQLevelTextBox_TextChanged(object sender, EventArgs e) {
			// 司令部レベルが書き換わった際は反映する
			FormFleet.lv = HQLevelTextBox.Text.ParseInt();
			RedrawSearchPower();
			file_state_modified(FileState.modified);
		}
		private void FleetTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetTypeComboBox.SelectedIndex == -1)
				return;
			// 艦隊形式が書き換わった際は反映する
			FormFleet.type = FleetTypeComboBox.SelectedIndex + 1;
			file_state_modified(FileState.modified);
		}
		private void FleetSelectComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(FleetSelectComboBox.SelectedIndex == -1)
				return;
			// 表示する艦隊を切り換える
			KammusuSelectListBox.DataSource = FormFleet.unit[FleetSelectComboBox.SelectedIndex];
		}
		private void KammusuSelectListBox_SelectedIndexChanged(object sender, EventArgs e) {
			var kammusu = (Kammusu)KammusuSelectListBox.SelectedItem;
			if (kammusu == null)
				return;
			// 表示する艦娘を切り替える
			int showKammusuType = kammusu.row.艦種 - 1;
			KammusuTypeComboBox.SelectedIndex = showKammusuType;
			KammusuNameComboBox.SelectedItem = kammusu;
			RedrawKammusuNameList();
			KammusuLevelTextBox.Text = kammusu.lv.ToString();
			KammusuLuckTextBox.Text = kammusu.luck.ToString();
			KammusuCondTextBox.Text = kammusu.cond.ToString();
			// 装備一覧を更新する
			WeaponSelectListBox.DataSource = kammusu.items;
		}
		private void KammusuTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawKammusuNameList();
		}
		private void WeaponTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawWeaponNameList();
		}
		private void WeaponSelectListBox_SelectedIndexChanged(object sender, EventArgs e) {
			var weapon = (Weapon)WeaponSelectListBox.SelectedItem;
			if (weapon == null)
				return;
			// 表示する装備を切り替える
			int showWeaponType;
			if (!WeaponTypeToNumber.TryGetValue(weapon.row.種別, out showWeaponType))
				showWeaponType = WeaponTypeToNumber["その他"];
			WeaponTypeComboBox.SelectedIndex = showWeaponType;
			WeaponNameComboBox.SelectedItem = weapon.row;
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
			if (SaveChangeBreforeNewCreate()) {
				if (MainTabControl.SelectedIndex == 0) {
					// ドラッグされたファイルを認識する
					string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
					if (files.Length < 1)
						return;
					FleetFilePath = files[0];
					// 拡張子で判別する
					if (Path.GetExtension(FleetFilePath) != ".json")
						return;
					// ファイルを詠みこみ、JSONとして解釈する
					FormFleet = Fleet.ReadFrom(FleetFilePath);
					if(FormFleet == null)
						return;
					// 読み込んだデータを画面に反映する
					file_state_modified(filepath_to_name(this.FleetFilePath), FileState.saved);
					HQLevelTextBox.Text = FormFleet.lv.ToString();
					FleetTypeComboBox.SelectedIndex = FormFleet.type - 1;
					FleetSelectComboBox_SelectedIndexChanged(sender, e);
					RedrawAntiAirScore();
					RedrawSearchPower();
				}
				else if (MainTabControl.SelectedIndex == 1) {
					// ドラッグされたファイルを認識する
					string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
					if (files.Length < 1)
						return;
					MapFilePath = files[0];
					// 拡張子で判別する
					if (Path.GetExtension(MapFilePath) != ".map")
						return;
					// ファイルを詠みこみ、JSONとして解釈する
					FormMapData = MapData.ReadFrom(MapFilePath);
					if (FormMapData == null)
						return;
					// 読み込んだデータを画面に反映する
					file_state_modified(filepath_to_name(this.MapFilePath), FileState.saved);
					MapPositionListBox.Items.Clear();
					foreach (var position in FormMapData.position) {
						MapPositionListBox.Items.Add(position.name);
					}
					MapPositionListBox.Refresh();
					RedrawMapAntiAirScore();
				}
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
			file_state_modified(FileState.modified);
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
			file_state_modified(FileState.modified);
		}
		private void DeleteMapPositionButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1)
				return;
			FormMapData.position.RemoveAt(MapPositionListBox.SelectedIndex);
			MapPositionListBox.Items.RemoveAt(MapPositionListBox.SelectedIndex);
			MapPositionListBox.Refresh();
			file_state_modified(FileState.modified);
		}
		private void AddMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternFormationComboBox.SelectedIndex == -1)
				return;
			// 艦隊を追加
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			selectPosition.pattern.Add(new Pattern { form = MapPatternFormationComboBox.SelectedIndex });
			// 画面上に反映
			int selectPositionCount = selectPosition.pattern.Count;
			MapPatternListBox.Items.Add(selectPositionCount.ToString() + " : " + selectPosition.pattern[selectPositionCount - 1].fleets.Count.ToString() + "隻");
			MapPatternListBox.Refresh();
			file_state_modified(FileState.modified);
		}
		private void ChangeMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternFormationComboBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			FormMapData.position[MapPositionListBox.SelectedIndex].pattern[MapPatternListBox.SelectedIndex].form = MapPatternFormationComboBox.SelectedIndex;
			file_state_modified(FileState.modified);
		}
		private void DeleteMapPatternButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			selectPosition.pattern.RemoveAt(MapPatternListBox.SelectedIndex);
			MapPatternListBox.Items.Clear();
			for(int fi = 0; fi < selectPosition.pattern.Count; ++fi) {
				MapPatternListBox.Items.Add((fi + 1).ToString() + " : " + selectPosition.pattern[fi].fleets.Count.ToString());
			}
			MapPatternListBox.Refresh();
			file_state_modified(FileState.modified);
		}
		private void AddMapKammusuButton_Click(object sender, EventArgs e) {
			var kammusu = (CsvDataSet.ShipsRow)MapKammusuNameComboBox.SelectedItem;
			if(kammusu == null
			|| MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			if(FormMapData.position[MapPositionListBox.SelectedIndex].pattern[MapPatternListBox.SelectedIndex].fleets.Count >= MaxUnitSize)
				return;
			// 艦娘データを作成
			var setKammusu = new Kammusu(kammusu.艦船ID);
			// 艦娘データを追加
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			var selectFleet = selectPosition.pattern[MapPatternListBox.SelectedIndex];
			int selectPositionCount = selectPosition.pattern.Count;
			selectFleet.fleets.Add(setKammusu);
			MapPatternListBox.Items[MapPatternListBox.SelectedIndex] = selectPositionCount.ToString() + " : " + selectFleet.fleets.Count.ToString() + "隻";
			RedrawMapAntiAirScore();
			file_state_modified(FileState.modified);
		}
		private void ChangeMapKammusuButton_Click(object sender, EventArgs e) {
			var kammusu = (CsvDataSet.ShipsRow)MapKammusuNameComboBox.SelectedItem;
			if(kammusu == null
			|| MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuListBox.SelectedIndex == -1)
				return;
			// 艦娘データを作成
			var setKammusu = new Kammusu(kammusu.艦船ID);
			// 艦娘データを追加
			var selectFleet = FormMapData.position[MapPositionListBox.SelectedIndex].pattern[MapPatternListBox.SelectedIndex];
			selectFleet.fleets[MapKammusuListBox.SelectedIndex] = setKammusu;
			RedrawMapAntiAirScore();
			file_state_modified(FileState.modified);
		}
		private void DeleteMapKammusuButton_Click(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1
			|| MapKammusuListBox.SelectedIndex == -1)
				return;
			FormMapData.position[MapPositionListBox.SelectedIndex].pattern[MapPatternListBox.SelectedIndex].fleets.RemoveAt(MapKammusuListBox.SelectedIndex);
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			var selectFleet = selectPosition.pattern[MapPatternListBox.SelectedIndex];
			int selectPositionCount = selectPosition.pattern.Count;
			MapPatternListBox.Items[MapPatternListBox.SelectedIndex] = selectPositionCount.ToString() + " : " + selectFleet.fleets.Count.ToString() + "隻";
			RedrawMapAntiAirScore();
			file_state_modified(FileState.modified);
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
			for(int pi = 0; pi < selectPosition.pattern.Count; ++pi) {
				MapPatternListBox.Items.Add((pi + 1).ToString() + " : " + selectPosition.pattern[pi].fleets.Count.ToString() + "隻");
			}
			MapPatternListBox.Refresh();
		}
		private void MapPatternListBox_SelectedIndexChanged(object sender, EventArgs e) {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			var selectPosition = FormMapData.position[MapPositionListBox.SelectedIndex];
			// 選択したパターンについて、その陣形に関する情報
			MapPatternFormationComboBox.SelectedIndex = selectPosition.pattern[MapPatternListBox.SelectedIndex].form;
			MapPatternFormationComboBox.Refresh();
			// 選択したパターンについて、それに含まれる艦娘に関する情報
			MapKammusuListBox.DataSource = selectPosition.pattern[MapPatternListBox.SelectedIndex].fleets;
			RedrawMapAntiAirScore();
		}
		private void MapKammusuListBox_SelectedIndexChanged(object sender, EventArgs e) {
			var kammusu = (Kammusu)MapKammusuListBox.SelectedItem;
			if (kammusu == null)
				return;
			// 表示する艦娘を切り替える
			MapKammusuTypeComboBox.SelectedIndex = kammusu.row.艦種 - 1;
			MapKammusuNameComboBox.SelectedItem = kammusu.row;
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
		// 装備データをGUIに反映
		private void RedrawWeaponNameList() {
			// 選択した種別に従って、リストを生成する
			var weapons = WeaponTypeComboBox.SelectedValue;
			if (weapons == null)
				return;
			WeaponNameComboBox.DataSource = weapons;
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
			// 選択した種別に従って、リストを生成する
			KammusuNameComboBox.DataSource = KammusuTypeComboBox.SelectedValue;
		}
		private void RedrawMapKammusuNameList() {
			// 選択した種別に従って、リストを生成する
			MapKammusuNameComboBox.DataSource = MapKammusuTypeComboBox.SelectedValue;
		}
		// 値を上下限で制限する
		static public int limit(int n, int min_n, int max_n) {
			return (n < min_n) ? min_n : (max_n < n) ? max_n : n;
		}
		// 制空値を計算して表示する(艦隊エディタ)
		private void RedrawAntiAirScore() {
			AllAntiAirTextBox.Text = CalcAntiAirScore(FormFleet.unit[0]).ToString();
		}
		// 索敵値を計算して表示する(艦隊エディタ)
		private void RedrawSearchPower() {
			double searchPower25A = CalcSearchPower25A(FormFleet.lv, FormFleet.unit[0]);
			double searchPower33 = CalcSearchPower33(FormFleet.lv, FormFleet.unit[0]);
			SearchPower25TextBox.Text = searchPower25A.ToString();
			SearchPower33TextBox.Text = searchPower33.ToString();
		}
		// 制空値を計算して表示する(マップエディタ)
		private void RedrawMapAntiAirScore() {
			if(MapPositionListBox.SelectedIndex == -1
			|| MapPatternListBox.SelectedIndex == -1)
				return;
			int antiAirScore = CalcAntiAirScore(FormMapData.position[MapPositionListBox.SelectedIndex].pattern[MapPatternListBox.SelectedIndex].fleets);
			MapPatternAllAntiAirTextBox.Text = antiAirScore.ToString();
		}
		// 制空値計算
		static int CalcAntiAirScore(IList<Kammusu> unit) {
			int antiAirScore = 0;
			foreach(var kammusu in unit) {
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
		static double CalcSearchPower25A(int level, IList<Kammusu> unit) {
			double searchPower = 0.0;
			// 司令部レベル補正
			int roundUp5Level = ((level - 1) / 5 + 1) * 5;
			searchPower += -0.6142467 * roundUp5Level;
			// 艦娘・装備による補正
			foreach(var kammusu in unit) {
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
		static double CalcSearchPower33(int level, IList<Kammusu> unit) {
			double searchPower = 0.0;
			// 司令部レベル補正
			searchPower -= Math.Ceiling(0.4 * level);
			// 艦娘・装備による補正
			foreach(var kammusu in unit) {
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
							searchPower += 1.2 * (searchValueW + 1.2 * Math.Sqrt(weapon.level));
							break;
						case "水上偵察機(夜偵)":
							searchPower += 1.2 * (searchValueW + 1.2 * Math.Sqrt(weapon.level));
							break;
						case "小型電探":
							searchPower += 0.6 * (searchValueW + 1.25 * Math.Sqrt(weapon.level));
							break;
						case "大型電探":
							searchPower += 0.6 * (searchValueW + 1.25 * Math.Sqrt(weapon.level));
							break;
						default:
							searchPower += 0.6 * searchValueW;
							break;
					}
				}
			}
			// 隻数による補正
			searchPower += 2.0 * (6.0 - unit.Count);
			// 小数第2位を四捨五入
			return 0.1 * Math.Round(10.0 * searchPower);
		}
		private string GetFilePath(int MainTabIndex) {
			if (!IsInRange(MainTabIndex, 0, 1)) throw new ArgumentOutOfRangeException("MainTabIndex");
			return (0 == MainTabIndex) ? MapFilePath : FleetFilePath;
		}
		private static DialogResult AskSaveChange(string filename) {
			return MessageBox.Show(filename + " has been modified, save changes ?",
				"Save Change?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button2
			);
		}
		private bool SaveChangeBeforeClear(int filename_index) {
			DialogResult result = AskSaveChange(this.file[MainTabControl.SelectedIndex].name);
			switch (result) {
			case DialogResult.Cancel:
				return false;
			case DialogResult.Yes:
				if (0 == MainTabControl.SelectedIndex) SaveFleetFile();
				else if (1 == MainTabControl.SelectedIndex) SaveMapFile();
				goto case DialogResult.No;
			case DialogResult.No:
				break;
			}
			return true;
		}
		private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
			for (int i = 0; i < this.file.Length; ++i) {
				if(FileState.modified == this.file[i].state && !SaveChangeBeforeClear(i)) e.Cancel = true;
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
	enum FileState{
		none,//ファイル作ってない
		new_created,//「新規」で作った直後
		modified,//どこかしら変更した
		saved//ファイルに保存されている(ファイル開いた直後or保存直後)
	}
	class OpenFileInfo {
		public OpenFileInfo() {
			this.name = "filename...";
			this.state = FileState.none;
			this.bg_color = SystemColors.Control;
		}
		public OpenFileInfo(string name_, FileState state_) {
			this.UpdateState(state_);
			this.name = name_;
		}
		public void UpdateState(FileState new_state) {
			this.state = new_state;
			switch (new_state) {
			case FileState.none:
				this.bg_color = SystemColors.Control;
				this.name = "filename...";
				break;
			case FileState.new_created:
				this.bg_color = SystemColors.Info;
				break;
			case FileState.modified:
				this.bg_color = Color.FromArgb(253, 239, 242);
				break;
			case FileState.saved:
				this.bg_color = Color.FromArgb(235, 246, 247);
				break;
			default:
				break;
			}
		}
		public string name;
		public FileState state;
		public Color bg_color;
	}
}
