#include "WebForge.hpp"
#include "../../Blam/Tags/TagInstance.hpp"
#include "../../Blam/Tags/TagBlock.hpp"
#include "../../Blam/Tags/Objects/Object.hpp"
#include "../../Blam/Tags/TagReference.hpp"
#include "../../Blam/Math/RealVector3D.hpp"
#include "../../Blam/BlamPlayers.hpp"
#include "../../Blam/BlamObjects.hpp"
#include "../../Blam/BlamTypes.hpp"
#include "../../Patches/Input.hpp"
#include "../../ThirdParty/rapidjson/writer.h"
#include "../../ThirdParty/rapidjson/document.h"
#include "../../ThirdParty/rapidjson/stringbuffer.h"
#include "../../Forge/ForgeUtil.hpp"
#include "ScreenLayer.hpp"
#include "../../ElDorito.hpp"
#include "../Bridge/WebRendererQuery.hpp"
#include "../../Pointer.hpp"
#include "../../Forge/Selection.hpp"
#include "../../Forge/ObjectSet.hpp"
#include <unordered_map>

using namespace Blam::Math;
using namespace Anvil::Client::Rendering::Bridge;

namespace
{
	enum class PropertyTarget
	{
		General_OnMapAtStart,
		General_Symmetry,
		General_RespawnRate,
		General_SpareClips,
		General_SpawnOrder,
		General_Team,
		General_TeleporterChannel,
		General_ShapeType,
		General_ShapeRadius,
		General_ShapeWidth,
		General_ShapeTop,
		General_ShapeBottom,
		General_ShapeDepth,
		General_Material,
		General_Physics,
		Budget_Minimum,
		Budget_Maximum,
		Light_ColorR,
		Light_ColorG,
		Light_ColorB,
		Light_ColorIntensity,
		Light_Intensity,
		Light_Radius,
		Fx_Hue,
		Fx_Saturation,
		Fx_TintR,
		Fx_TintG,
		Fx_TintB,
		Fx_ColorMute,
		Fx_LightIntensity,
		Fx_Darkness,
		Fx_Brightness,
		Fx_Range
	};

	enum class PropertyDataType
	{
		Int = 0,
		Float
	};

	struct PropertyValue
	{
		union
		{
			float ValueFloat;
			int ValueInt;
		};
	};

	struct PropertyInfo
	{
		PropertyDataType Type;
		PropertyTarget Target;
	};

	class ObjectPropertySetter
	{
	public:
		ObjectPropertySetter(const Blam::MapVariant::VariantProperties& properties, const Blam::MapVariant::BudgetEntry& budget)
			: m_Properties(properties), m_Budget(budget), m_RespawnRequired(false), m_BudgetDirty(false) {}

		void SetProperty(PropertyTarget target, PropertyValue value)
		{
			switch (target)
			{
			case PropertyTarget::General_OnMapAtStart:
				break;
			case PropertyTarget::General_Symmetry:
				SetSymmetry(value.ValueInt);
				break;
			case PropertyTarget::General_RespawnRate:
				m_Properties.RespawnTime = value.ValueInt;
				break;
			case PropertyTarget::General_SpareClips:
				m_Properties.SharedStorage = value.ValueInt;
				break;
			case PropertyTarget::General_SpawnOrder:
				m_Properties.SharedStorage = value.ValueInt;
				break;
			case PropertyTarget::General_Team:
				m_Properties.TeamAffilation = value.ValueInt;
				break;
			case PropertyTarget::General_TeleporterChannel:
				m_Properties.SharedStorage = value.ValueInt;
				break;
			case PropertyTarget::General_ShapeType:
				m_Properties.ZoneShape = value.ValueInt;
				break;
			case PropertyTarget::General_ShapeRadius:
				m_Properties.ZoneRadiusWidth = value.ValueFloat;
				break;
			case PropertyTarget::General_ShapeWidth:
				m_Properties.ZoneRadiusWidth = value.ValueFloat;
				break;
			case PropertyTarget::General_ShapeTop:
				m_Properties.ZoneTop = value.ValueFloat;
				break;
			case PropertyTarget::General_ShapeBottom:
				m_Properties.ZoneBottom = value.ValueFloat;
				break;
			case PropertyTarget::General_ShapeDepth:
				m_Properties.ZoneDepth = value.ValueFloat;
				break;
			case PropertyTarget::General_Material:
				m_Properties.SharedStorage = value.ValueInt;
				break;
			case PropertyTarget::General_Physics:
				SetPhysics(value.ValueInt);
				break;
			case PropertyTarget::Budget_Minimum:
				SetBudgetMinimum(value.ValueInt);
				break;
			case PropertyTarget::Budget_Maximum:
				SetBudgetMinimum(value.ValueInt);
				break;
			case PropertyTarget::Light_ColorR:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->ColorR = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Light_ColorG:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->ColorG = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Light_ColorB:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->ColorB = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Light_ColorIntensity:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->ColorIntensity = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Light_Intensity:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->Intensity = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Light_Radius:
				reinterpret_cast<Forge::ForgeLightProperties*>(&m_Properties.ZoneRadiusWidth)->Range = value.ValueFloat;
				break;
			case PropertyTarget::Fx_TintR:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->TintR = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_TintG:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->TintG = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_TintB:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->TintB = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_ColorMute:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->ColorMuting = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_Brightness:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->Brightness = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_Darkness:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->Darkness = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_Hue:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->Hue = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_LightIntensity:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->LightIntensity = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_Saturation:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->Saturation = int(value.ValueFloat * 255);
				break;
			case PropertyTarget::Fx_Range:
				reinterpret_cast<Forge::ForgeScreenFxProperties*>(&m_Properties.ZoneRadiusWidth)->Range = value.ValueInt;
				break;
			}
		}

		bool RespawnRequired() const
		{
			return m_RespawnRequired;
		}

		bool SyncRequired() const
		{
			return m_SyncRequired;
		}

		void SetSymmetry(int value)
		{
			switch (value)
			{
			case 2:
				m_Properties.ObjectFlags = m_Properties.ObjectFlags & ~4u | 8u;
				break;
			case 1:
				m_Properties.ObjectFlags = m_Properties.ObjectFlags & ~8u | 4u;
				break;
			case 0:
				m_Properties.ObjectFlags |= 0xCu;
				break;
			}
		}

		void SetBudgetMinimum(int value)
		{
			m_Budget.RuntimeMin = value;
			m_BudgetDirty = true;
		}

		void SetBudgetMaximum(int value)
		{
			m_Budget.RuntimeMax = value;
			m_BudgetDirty = true;
		}

		void SetPhysics(int value)
		{
			switch (value)
			{
			case 0:
				m_Properties.ZoneShape = 0;
				break;
			case 1:
				m_Properties.ZoneShape = 4;
				break;
			}

		}

		void Apply(uint32_t playerIndex, int16_t placementIndex)
		{
			static auto Forge_SetPlacementVariantProperties = (void(*)(uint32_t playerIndex,
				int placementIndex, Blam::MapVariant::VariantProperties *properties))(0x0059B720);

			Forge_SetPlacementVariantProperties(playerIndex, placementIndex, &m_Properties);

			if (m_BudgetDirty)
			{
				Forge::ForgeMessage msg;
				msg.Type = 6;
				msg.PlayerIndex = playerIndex;
				msg.TagIndex = m_Budget.TagIndex;
				msg.QuotaMin = m_Budget.RuntimeMin;
				msg.QuotaMax = m_Budget.RuntimeMax;

				static auto Forge_SendMessage = (void(*)(Forge::ForgeMessage*))(0x004735D0);
				Forge_SendMessage(&msg);
			}
		}

	private:
		Blam::MapVariant::VariantProperties m_Properties;
		Blam::MapVariant::BudgetEntry m_Budget;
		bool m_Valid;
		bool m_RespawnRequired;
		bool m_SyncRequired;
		bool m_BudgetDirty;
	};

	struct IObjectPropertySink
	{
		virtual void SetProperty(PropertyTarget target, PropertyValue value) = 0;
		virtual ~IObjectPropertySink() {}
	};

	class DeferedPropertySink : public IObjectPropertySink
	{
	public:
		void SetProperty(PropertyTarget target, PropertyValue value) override
		{
			m_Values.emplace_back(target, value);
		}

		void Apply(uint32_t playerIndex, int16_t placementIndex)
		{
			auto mapv = Forge::GetMapVariant();
			if (!mapv || placementIndex == -1)
				return;

			const auto &placement = mapv->Placements[placementIndex];
			ObjectPropertySetter propertySetter(placement.Properties, mapv->Budget[placement.BudgetIndex]);
			for (auto& v : m_Values)
				propertySetter.SetProperty(std::get<0>(v), std::get<1>(v));

			propertySetter.Apply(playerIndex, placementIndex);
			m_Values.clear();
		}

	private:
		std::vector<std::tuple<PropertyTarget, PropertyValue>> m_Values;
	};

	class ImmediatePropertySink : public IObjectPropertySink
	{
	public:

		ImmediatePropertySink(uint32_t playerIndex, int16_t placementIndex) :
			m_PlayerIndex(playerIndex), m_PlacementIndex(placementIndex) {}

		void SetProperty(PropertyTarget target, PropertyValue value) override
		{
			auto mapv = Forge::GetMapVariant();
			if (!mapv)
				return;

			const auto &placement = mapv->Placements[m_PlacementIndex];
			ObjectPropertySetter propertySetter(placement.Properties, mapv->Budget[placement.BudgetIndex]);
			propertySetter.SetProperty(target, value);
			propertySetter.Apply(m_PlayerIndex, m_PlacementIndex);
		}

	private:
		uint32_t m_PlayerIndex;
		int16_t m_PlacementIndex;
	};

	std::string SerializeObjectProperties(int16_t placementIndex);
	void DeserializeObjectProperties(const rapidjson::Value &json, IObjectPropertySink& sink);

	void SerializeProperty(rapidjson::Writer<rapidjson::StringBuffer>& writer, const char* key, int value) { writer.Key(key); writer.Int(value); };
	void SerializeProperty(rapidjson::Writer<rapidjson::StringBuffer>& writer, const char* key, float value) { writer.Key(key); writer.Double(value); };
	void SerializeProperty(rapidjson::Writer<rapidjson::StringBuffer>& writer, const char* key, bool value) { writer.Key(key); writer.Bool(value); };

	uint32_t s_CurrentObjectIndex = -1;
	DeferedPropertySink s_ItemSpawnProperties;
}

namespace Web::Ui::WebForge
{
	void OnItemSpawned(uint32_t objectIndex)
	{
		auto object = Blam::Objects::Get(objectIndex);
		if (!object || object->PlacementIndex == -1)
			return;

		s_ItemSpawnProperties.Apply(Blam::Players::GetLocalPlayer(0), object->PlacementIndex);
	}

	void ShowObjectProperties(uint32_t objectIndex)
	{
		auto currentObject = Blam::Objects::Get(objectIndex);
		if (currentObject && currentObject->PlacementIndex != 0xFFFF)
		{
			auto mapv = Forge::GetMapVariant();
			auto placement = mapv->Placements[currentObject->PlacementIndex];

			s_CurrentObjectIndex = objectIndex;

			Web::Ui::ScreenLayer::Show("forge_object_properties", SerializeObjectProperties(currentObject->PlacementIndex));
		}
	}

	void ShowObjectCreation()
	{
		const auto mapv = Forge::GetMapVariant();
		if (!mapv)
			return;

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		char buff[256];

		writer.StartObject();
		writer.Key("budget");
		writer.StartArray();
		for (auto i = 0; i < mapv->BudgetEntryCount; i++)
		{
			const auto& itemBudget = mapv->Budget[i];

			sprintf_s(buff, 256, "0x%x", itemBudget.TagIndex);

			writer.StartObject();
			writer.Key("tagindex");
			writer.String(buff);
			SerializeProperty(writer, "max_allowed", itemBudget.DesignTimeMax);
			SerializeProperty(writer, "count_on_map", itemBudget.CountOnMap);
			SerializeProperty(writer, "runtime_min", itemBudget.RuntimeMin);
			SerializeProperty(writer, "runtime_max", itemBudget.RuntimeMax);
			writer.EndObject();
		}
		writer.EndArray();
		writer.EndObject();

		Web::Ui::ScreenLayer::Show("forge_object_creation", buffer.GetString());
	}

	QueryError ProcessAction(const rapidjson::Value &p_Args, std::string *p_Result)
	{
		auto type = p_Args.FindMember("type");
		auto data = p_Args.FindMember("data");
		if (type == p_Args.MemberEnd() || !type->value.IsNumber())
		{
			*p_Result = "Bad query : A \"type\" argument is required and must be a number";
			return QueryError_BadQuery;
		}
		if (data == p_Args.MemberEnd() || !data->value.IsObject())
		{
			*p_Result = "Bad query : \"data\" argument is required and must be a object";
			return QueryError_BadQuery;
		}

		switch (type->value.GetInt())
		{
		case 1: // set immediate properties
		{
			auto currentObject = Blam::Objects::Get(s_CurrentObjectIndex);
			if (currentObject && currentObject->PlacementIndex != 0xFFFF)
			{
				auto mapv = Forge::GetMapVariant();
				auto playerIndex = Blam::Players::GetLocalPlayer(0);
				ImmediatePropertySink sink(playerIndex, currentObject->PlacementIndex);
				DeserializeObjectProperties(data->value, sink);
			}
		}
		break;
		case 2: // set defered properties
			DeserializeObjectProperties(data->value, s_ItemSpawnProperties);
			break;
		}

		return QueryError_Ok;
	}
}

namespace
{

	bool TryParseInt(const std::string& str, int* value)
	{
		if (str.length() == 0)
			return false;

		auto c_str = str.c_str();
		char* endp;

		*value = std::strtol(c_str, &endp, 10);

		return endp != c_str;
	}

	bool TryParseByte(const std::string& str, uint8_t* value)
	{
		int intval;
		if (!TryParseInt(str, &intval))
			return false;

		*value = static_cast<uint8_t>(intval);
		return true;
	}

	bool TryParseFloat(const std::string& str, float* value)
	{
		const auto c_str = str.c_str();
		char* endp;
		*value = static_cast<float>(std::strtod(c_str, &endp));
		return c_str != endp;
	}

	bool CanThemeObject()
	{
		const auto FIRST_THEMEABLE_SHADER_TAG_INDEX = 0x3ab0;

		auto object = Blam::Objects::Get(s_CurrentObjectIndex);
		if (!object)
			return false;

		auto objectDef = Blam::Tags::TagInstance(object->TagIndex).GetDefinition<uint8_t>();
		if (!objectDef)
			return false;
		auto hlmtDef = Blam::Tags::TagInstance(*(uint32_t*)(objectDef + 0x40)).GetDefinition<uint8_t>();
		if (!hlmtDef)
			return false;
		auto modeTagIndex = *(uint32_t*)(hlmtDef + 0xC);

		const auto modeDefinitionPtr = Pointer(Blam::Tags::TagInstance(modeTagIndex).GetDefinition<uint8_t>());
		if (!modeDefinitionPtr)
			return false;

		const auto materialCount = modeDefinitionPtr(0x48).Read<int32_t>();
		const auto& firstMaterialShaderTagRef = modeDefinitionPtr(0x4c)[0].Read<Blam::Tags::TagReference>();
		if (!materialCount || firstMaterialShaderTagRef.TagIndex != FIRST_THEMEABLE_SHADER_TAG_INDEX)
			return false;

		return true;
	}

	bool IsForgeLight(uint32_t objectIndex)
	{
		auto object = Blam::Objects::Get(objectIndex);
		if (!object)
			return false;
		auto objectDef = Blam::Tags::TagInstance(object->TagIndex).GetDefinition<Blam::Tags::Objects::Object>();
		if (!objectDef)
			return false;

		for (const auto& attachment : objectDef->Attachments)
		{
			if (attachment.Attached.GroupTag != 'ligh')
				continue;

			auto lightFlags = *(uint32_t*)Blam::Tags::TagInstance(attachment.Attached.TagIndex).GetDefinition<uint8_t>();
			if (lightFlags & (1 << 31))
				return true;
		}

		return false;
	}

	bool IsForgeScreenEffectObject(uint32_t objectIndex)
	{
		auto object = Blam::Objects::Get(objectIndex);
		if (!object)
			return false;
		auto objectDef = Blam::Tags::TagInstance(object->TagIndex).GetDefinition<Blam::Tags::Objects::Object>();
		if (!objectDef)
			return false;

		for (const auto& attachment : objectDef->Attachments)
		{
			if (attachment.Attached.GroupTag != 'effe')
				continue;

			if (attachment.Marker == 0x13BB)
				return true;
		}

		return false;
	}

	std::string SerializeObjectProperties(int16_t placementIndex)
	{
		static auto Weapon_HasSpareClips = (bool(*)(uint32_t tagIndex))(0x00B624E0);

		auto mapv = Forge::GetMapVariant();
		auto placement = mapv->Placements[placementIndex];
		const auto& properties = placement.Properties;
		const auto& budget = mapv->Budget[placement.BudgetIndex];

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		auto symmetry = 0;
		if (properties.ObjectFlags & 4)
		{
			if (!(properties.ObjectFlags & 8))
				symmetry = 1;
		}
		else
			symmetry = 2;

		auto lightProperties = reinterpret_cast<const Forge::ForgeLightProperties*>(&properties.ZoneRadiusWidth);
		auto screenFxProperties = reinterpret_cast<const Forge::ForgeScreenFxProperties*>(&properties.ZoneRadiusWidth);

		writer.StartObject();
		SerializeProperty(writer, "object_type_mp", properties.ObjectType);
		SerializeProperty(writer, "has_material", CanThemeObject());
		SerializeProperty(writer, "has_spare_clips", properties.ObjectType == 1 && !Weapon_HasSpareClips(budget.TagIndex));
		SerializeProperty(writer, "is_selected", Forge::Selection::GetSelection().Contains(placement.ObjectIndex));
		SerializeProperty(writer, "is_light", IsForgeLight(placement.ObjectIndex));
		SerializeProperty(writer, "is_screenfx", IsForgeScreenEffectObject(placement.ObjectIndex));

		writer.Key("properties");
		writer.StartObject();
		SerializeProperty(writer, "on_map_at_start", ((properties.ObjectFlags >> 1) & 1) == 0 ? 1 : 0);
		SerializeProperty(writer, "symmetry", symmetry);
		SerializeProperty(writer, "respawn_rate", properties.RespawnTime);
		SerializeProperty(writer, "spare_clips", properties.SharedStorage);
		SerializeProperty(writer, "team_affiliation", properties.TeamAffilation);
		SerializeProperty(writer, "teleporter_channel", properties.SharedStorage);
		SerializeProperty(writer, "shape_type", properties.ZoneShape);
		SerializeProperty(writer, "shape_radius", properties.ZoneRadiusWidth);
		SerializeProperty(writer, "shape_top", properties.ZoneTop);
		SerializeProperty(writer, "shape_bottom", properties.ZoneBottom);
		SerializeProperty(writer, "shape_width", properties.ZoneRadiusWidth);
		SerializeProperty(writer, "shape_depth", properties.ZoneDepth);
		SerializeProperty(writer, "appearance_material", properties.SharedStorage);
		SerializeProperty(writer, "physics", properties.ZoneShape == 4 ? 1 : 0);
		SerializeProperty(writer, "light_color_b", lightProperties->ColorB / 255.0f);
		SerializeProperty(writer, "light_color_g", lightProperties->ColorG / 255.0f);
		SerializeProperty(writer, "light_color_r", lightProperties->ColorR / 255.0f);
		SerializeProperty(writer, "light_color_intensity", lightProperties->ColorIntensity / 255.0f);
		SerializeProperty(writer, "light_intensity", lightProperties->Intensity / 255.0f);
		SerializeProperty(writer, "light_radius", lightProperties->Range);
		SerializeProperty(writer, "fx_tint_r", screenFxProperties->TintR / 255.0f);
		SerializeProperty(writer, "fx_tint_g", screenFxProperties->TintG / 255.0f);
		SerializeProperty(writer, "fx_tint_b", screenFxProperties->TintB / 255.0f);
		SerializeProperty(writer, "fx_hue",	screenFxProperties->Hue / 255.0f);
		SerializeProperty(writer, "fx_saturation", screenFxProperties->Saturation / 255.0f);
		SerializeProperty(writer, "fx_color_mute", screenFxProperties->ColorMuting / 255.0f);
		SerializeProperty(writer, "fx_light_intensity", screenFxProperties->LightIntensity / 255.0f);
		SerializeProperty(writer, "fx_brightness", screenFxProperties->Brightness / 255.0f);
		SerializeProperty(writer, "fx_darkness", screenFxProperties->Darkness / 255.0f);
		SerializeProperty(writer, "fx_range", screenFxProperties->Range);
		writer.EndObject();

		writer.Key("budget");
		writer.StartObject();
		SerializeProperty(writer, "summary_placed_on_map", budget.CountOnMap);
		SerializeProperty(writer, "summary_total_cost", budget.Cost * budget.CountOnMap);
		SerializeProperty(writer, "summary_maximum_allowed", budget.DesignTimeMax);
		SerializeProperty(writer, "summary_runtime_minimum", budget.RuntimeMin);
		SerializeProperty(writer, "summary_runtime_maximum", budget.RuntimeMax);
		writer.EndObject();

		writer.EndObject();
		return buffer.GetString();
	}

	void DeserializeObjectProperties(const rapidjson::Value &json, IObjectPropertySink& sink)
	{
		const static std::unordered_map<std::string, PropertyInfo> s_PropertyTypeLookup =
		{
			{ "on_map_at_start",			{ PropertyDataType::Int, PropertyTarget::General_OnMapAtStart }},
			{ "symmetry",					{ PropertyDataType::Int, PropertyTarget::General_Symmetry }},
			{ "respawn_rate",				{ PropertyDataType::Int, PropertyTarget::General_RespawnRate }},
			{ "spare_clips",				{ PropertyDataType::Int, PropertyTarget::General_SpareClips }},
			{ "spawn_order",				{ PropertyDataType::Int, PropertyTarget::General_SpawnOrder }},
			{ "team_affiliation",			{ PropertyDataType::Int, PropertyTarget::General_Team }},
			{ "physics",					{ PropertyDataType::Int, PropertyTarget::General_Physics } },
			{ "appearance_material",		{ PropertyDataType::Int, PropertyTarget::General_Material } },
			{ "teleporter_channel",			{ PropertyDataType::Int, PropertyTarget::General_TeleporterChannel }},
			{ "shape_type",					{ PropertyDataType::Int, PropertyTarget::General_ShapeType }},
			{ "shape_radius",				{ PropertyDataType::Float, PropertyTarget::General_ShapeRadius}},
			{ "shape_width",				{ PropertyDataType::Float, PropertyTarget::General_ShapeWidth }},
			{ "shape_top",					{ PropertyDataType::Float, PropertyTarget::General_ShapeTop }},
			{ "shape_bottom",				{ PropertyDataType::Float, PropertyTarget::General_ShapeBottom }},
			{ "shape_depth",				{ PropertyDataType::Float, PropertyTarget::General_ShapeDepth }},
			{ "light_color_r",				{ PropertyDataType::Float, PropertyTarget::Light_ColorR }},
			{ "light_color_g",				{ PropertyDataType::Float, PropertyTarget::Light_ColorG }},
			{ "light_color_b",				{ PropertyDataType::Float, PropertyTarget::Light_ColorB }},
			{ "light_color_intensity",		{ PropertyDataType::Float, PropertyTarget::Light_ColorIntensity }},
			{ "light_intensity",			{ PropertyDataType::Float, PropertyTarget::Light_Intensity }},
			{ "light_radius",				{ PropertyDataType::Float, PropertyTarget::Light_Radius }},
			{ "fx_hue",						{ PropertyDataType::Float, PropertyTarget::Fx_Hue}},
			{ "fx_saturation",				{ PropertyDataType::Float, PropertyTarget::Fx_Saturation } },
			{ "fx_tint_r",					{ PropertyDataType::Float, PropertyTarget::Fx_TintR } },
			{ "fx_tint_g",					{ PropertyDataType::Float, PropertyTarget::Fx_TintG } },
			{ "fx_tint_b",					{ PropertyDataType::Float, PropertyTarget::Fx_TintB } },
			{ "fx_color_mute",				{ PropertyDataType::Float, PropertyTarget::Fx_ColorMute } },
			{ "fx_light_intensity",			{ PropertyDataType::Float, PropertyTarget::Fx_LightIntensity } },
			{ "fx_darkness",				{ PropertyDataType::Float, PropertyTarget::Fx_Darkness } },
			{ "fx_brightness",				{ PropertyDataType::Float, PropertyTarget::Fx_Brightness } },
			{ "fx_range",					{ PropertyDataType::Int, PropertyTarget::Fx_Range } },
			{ "summary_runtime_minimum",	{ PropertyDataType::Int, PropertyTarget::Budget_Minimum } },
			{ "summary_runtime_maximum",	{ PropertyDataType::Int, PropertyTarget::Budget_Maximum } },
		};

		for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
		{
			const auto& name = it->name.GetString();

			auto typeIt = s_PropertyTypeLookup.find(name);
			if (typeIt == s_PropertyTypeLookup.end())
				continue;

			auto info = typeIt->second;

			PropertyValue value;
			switch (info.Type)
			{
			case PropertyDataType::Int:
				value.ValueInt = it->value.GetInt();
				break;
			case PropertyDataType::Float:
				value.ValueFloat = it->value.GetDouble();
				break;
			}

			sink.SetProperty(info.Target, value);
		}
	}
}