using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using static KCS_GUI.CsvDataSet;

namespace KCS_GUI {
	// 装備
	[JsonObject(MemberSerialization = MemberSerialization.OptIn)]
	class Weapon {
		public readonly CsvDataSet.WeaponsRow row;
		readonly bool isAir;
		int level_;
		int rf_;
		int rf_detail_;

		// 装備ID
		[JsonProperty]
		public int id { get { return row.装備ID; } }

		public string 装備名 { get { return row.装備名; } }

		// rfのSerialzeに使用する
		[JsonProperty("rf"), EditorBrowsable(EditorBrowsableState.Never)]
		public int rf_or_level {
			get { return isAir ? rf : level; }
		}

		// 装備改修度
		public int level {
			get { return level_; }
			set { level_ = value.limit(0, 10); }
		}

		// 外部熟練度
		public int rf {
			get { return rf_; }
			set {
				rf_ = value.limit(0, 7);
				rf_detail_ = ToDetail(rf_);
			}
		}

		// 内部熟練度
		[JsonProperty]
		public int rf_detail {
			get { return rf_detail_; }
			set {
				rf_detail_ = value.limit(0, 120);
				rf_ = FromDetail(rf_detail_);
			}
		}

		public Weapon(int id) {
			// IDがデータベースに存在するか判定
			row = data.Weapons.SingleOrDefault(w => w.装備ID == id);
			if (row == null)
				throw new ArgumentOutOfRangeException("id");
			// 種別を判定することで、"rf"が装備改修度か艦載機熟練度かを判別する
			int setWeaponType;
			if (!WeaponTypeToNumber.TryGetValue(row.種別, out setWeaponType))
				setWeaponType = WeaponTypeToNumber["その他"];
			isAir = RfWeaponTypeList.Contains(setWeaponType);
		}

		public Weapon(int id, int level, int rf, int rf_detail) : this(id) {
			this.level = level;
			this.rf_ = rf.limit(0, 7);
			this.rf_detail_ = rf_detail.limit(0, 120);
		}

		[JsonConstructor]
		public Weapon(int id, int rf, int? rf_detail) : this(id) {
			if (isAir) {
				if (rf_detail == null)
					this.rf = rf;
				else
					this.rf_detail = rf_detail.Value;
			} else
				this.level = rf;
		}

		// JSONにrf_detailを書き出すか、Serializerが使用する
		[EditorBrowsable(EditorBrowsableState.Never)]
		public bool ShouldSerializerf_detail() {
			return isAir && ToDetail(rf) != rf_detail;
		}

		// 外部熟練度を内部熟練度に変換する
		public static int ToDetail(int rf) {
			return (rf * 15 - 5).limit(0, 120);
		}

		// 内部熟練度を外部熟練度に変換する
		public static int FromDetail(int rf_detail) {
			return ((rf_detail + 5) / 15).limit(0, 7);
		}
	}
	// 艦娘
	class Kammusu {
		[JsonIgnore]
		public readonly CsvDataSet.ShipsRow row;
		int lv_;
		int luck_;
		int cond_;

		// 艦船ID
		public int id { get { return row.艦船ID; } }

		[JsonIgnore]
		public string 艦名 { get { return row.艦名; } }

		// レベル
		public int lv {
			get { return lv_; }
			set { lv_ = value.limit(1, 155); }
		}

		// 運
		// デッキビルダーの仕様上、"-1"もOK
		public int luck {
			get { return luck_; }
			set { luck_ = value.limit(-1, 100); }
		}

		// cond値
		[JsonProperty(DefaultValueHandling = DefaultValueHandling.Ignore), DefaultValue(49)]
		public int cond {
			get { return cond_; }
			set { cond_ = value.limit(0, 100); }
		}

		// 装備
		[JsonConverter(typeof(ListJsonConverter<Weapon>), "i")]
		public IList<Weapon> items { get; set; }

		// 最大スロット数
		[JsonIgnore]
		public int maxSlots { get { return row.スロット数; } }

		[JsonConstructor]
		public Kammusu(int id, int lv = 1, int luck = -1, int cond = 49, [JsonConverter(typeof(ListJsonConverter<Weapon>), "i")] IList<Weapon> items = null) {
			row = data.Ships.SingleOrDefault(s => s.艦船ID == id);
			if (row == null)
				throw new ArgumentOutOfRangeException("id");
			this.lv = lv;
			this.luck = luck;
			this.cond = cond;
			this.items = items ?? new BindingList<Weapon>(
				row.初期装備
					.Split('/')
					.Select(Int32.Parse)
					.Where(weaponID => 0 < weaponID)
					.Select(weaponID => new Weapon(weaponID))
					.ToList()
			);
		}
	}
	// 艦隊
	class Fleet {
		//艦隊数の最大
		const int MaxFleetSize = 2;
		static JsonSerializer serailzer = new JsonSerializer { Converters = { new ListJsonConverter<Kammusu>("s") } };

		int lv_ = 120;
		int type_ = 0;

		public int version { get; set; } = 3;
		// 司令部レベル
		public int lv {
			get { return lv_; }
			set { lv_ = value.limit(1, 120); }
		}
        // 艦隊形式、プログラム内では0～3の値を取る
        [JsonIgnore]
        public int type {
			get { return type_; }
			set { type_ = value.limit(0, 3); }
		}
        // 艦隊形式、JSON表現では+1して1～4の値を取る
        [JsonProperty("type", DefaultValueHandling = DefaultValueHandling.Ignore), DefaultValue(1), EditorBrowsable(EditorBrowsableState.Never)]
        public int json_type {
            get { return type + 1; }
            set { type = value - 1; }
        }
        // 艦娘
        [JsonIgnore]
		public IList<BindingList<Kammusu>> unit = Enumerable.Range(0, MaxFleetSize).Select(_ => new BindingList<Kammusu>()).ToList();

		[JsonExtensionData]
		IDictionary<string, JToken> additionalData = new Dictionary<string, JToken>();
		[OnDeserialized]
		void OnDeserialized(StreamingContext context) {
			for (int i = 0; i < MaxFleetSize; ++i) {
				JToken token;
				if (!additionalData.TryGetValue($"f{i + 1}", out token))
					break;
				unit[i] = token.ToObject<BindingList<Kammusu>>(serailzer);
			}
			additionalData.Clear();
		}
		[OnSerializing]
		void OnSerializing(StreamingContext context) {
			additionalData.Clear();
			// 書き出す艦隊数は艦隊形式によって制御する
			int writeFleets = type == 0 ? 1 : 2;
			for (int fi = 0; fi < writeFleets; ++fi) {
				// 艦隊
				additionalData[$"f{fi + 1}"] = JToken.FromObject(unit[fi], serailzer);
			}
		}

		public static Fleet ReadFrom(string path) {
			try {
				return JsonConvert.DeserializeObject<Fleet>(File.ReadAllText(path));
			}
			catch {
				return null;
			}
		}

		// JSON書き出し
		public void WriteTo(string path) {
			File.WriteAllText(path, JsonConvert.SerializeObject(this));
		}
	}
	class Pattern {
		public int form;
		[JsonProperty(ItemConverterType = typeof(KammusuIdJsonConverter))]
		public IList<Kammusu> fleets = new BindingList<Kammusu>();
	}
	// マスデータ
	class Position {
		// マスにおける戦闘モード
		public int mode;
		// マスの名称
		public string name;
		// 各パターン
		public IList<Pattern> pattern = new BindingList<Pattern>();
	}
	// マップデータ
	class MapData {
		public string version = "map";
		// マス
		public IList<Position> position = new BindingList<Position>();

		public static MapData ReadFrom(string path) {
			try {
				return JsonConvert.DeserializeObject<MapData>(File.ReadAllText(path));
			}
			catch {
				return null;
			}
		}
		// JSON書き出し
		public void WriteTo(string path) {
			File.WriteAllText(path, JsonConvert.SerializeObject(this));
		}
	}
	public class ListJsonConverter<T> : JsonConverter {
		readonly string prefix;

		public ListJsonConverter(string prefix) {
			this.prefix = prefix;
		}

		public override bool CanConvert(Type objectType) {
			return typeof(IList<T>).IsAssignableFrom(objectType);
		}

		public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer) {
			var dictionary = serializer.Deserialize<Dictionary<string, T>>(reader);
			var list = (serializer.ObjectCreationHandling != ObjectCreationHandling.Replace ? existingValue as IList<T> : null) ?? new BindingList<T>();
			for (var i = 1; ; i++) {
				T t;
				if (!dictionary.TryGetValue(prefix + i, out t))
					break;
				list.Add(t);
			}
			return list;
		}

		public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) {
			serializer.Serialize(writer, ((IList<T>)value)?.Select((t, i) => new { k = $"{prefix}{i + 1}", t })?.ToDictionary(a => a.k, a => a.t));
		}
	}
	public class KammusuIdJsonConverter : JsonConverter {
		public override bool CanConvert(Type objectType) {
			return objectType == typeof(Kammusu);
		}

		public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer) {
			return new Kammusu(serializer.Deserialize<int>(reader));
		}

		public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) {
			serializer.Serialize(writer, ((Kammusu)value).id);
		}
	}
}
