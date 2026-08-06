#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

bool g_enableObjectSpawn = false;

class $modify(MyEditorLayer, LevelEditorLayer) {
    bool isPlaying() {
        return m_playbackMode == PlaybackMode::Playing;
    }
};

void placeCustomObject(PlayerObject* player, int holdState) {
    if (!g_enableObjectSpawn || !player || !player->m_editorEnabled)
        return;

    auto editor = GameManager::sharedState()->getEditorLayer();
    if (!editor) return;

    auto myEditor = static_cast<MyEditorLayer*>(editor);
    if (!myEditor || !myEditor->isPlaying())
        return;

    auto pos = player->getPosition();
    pos.y -= 90.f;

    int settingVal = Mod::get()->getSettingValue<int>("custom-setting-value");

    int playerNumber = 165;
    if (player->m_isSecondPlayer)
        playerNumber = 199;

    std::string objStr = fmt::format(
        "1,2899,2,{},3,{},{},{}",
        pos.x, pos.y, playerNumber, holdState
    );

    if (settingVal > 0)
        objStr += fmt::format(",33,{}", settingVal);

    editor->createObjectsFromString(objStr, false, true);
}

class $modify(MyPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        auto ret = PlayerObject::pushButton(btn);

        if (!m_editorEnabled || !g_enableObjectSpawn)
            return ret;

        if (btn == PlayerButton::Jump)
            placeCustomObject(this, -1);

        return ret;
    }

    bool releaseButton(PlayerButton btn) {
        auto ret = PlayerObject::releaseButton(btn);

        if (!m_editorEnabled || !g_enableObjectSpawn)
            return ret;

        if (btn == PlayerButton::Jump)
            placeCustomObject(this, 1);

        return ret;
    }
};

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        CCMenuItemSpriteExtra* m_toggleBtn = nullptr;
    };

    void updateButtonState() {
        if (!m_fields->m_toggleBtn) return;

        auto spr = ButtonSprite::create(
            "Auto\nOptions",
            25,
            true,
            "bigFont.fnt",
            "GJ_button_01.png",
            40.f,
            0.6f
        );

        if (g_enableObjectSpawn)
            spr->setColor({255, 255, 255});
        else
            spr->setColor({100, 100, 100});

        m_fields->m_toggleBtn->setNormalImage(spr);
    }

    void onToggleButton(CCObject*) {
        g_enableObjectSpawn = !g_enableObjectSpawn;
        updateButtonState();
    }

    void visibilityUpdate(float dt) {
        if (!m_fields->m_toggleBtn) return;

        if (m_editorLayer) {
            auto myEditor = static_cast<MyEditorLayer*>(m_editorLayer);
            if (myEditor && myEditor->isPlaying()) {
                m_fields->m_toggleBtn->setVisible(false);
                return;
            }
        }
        m_fields->m_toggleBtn->setVisible(true);
    }

    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor))
            return false;

        g_enableObjectSpawn = false;

        bool hideBtn = Mod::get()->getSettingValue<bool>("hideBtn");

        if (!hideBtn) {
            auto spr = ButtonSprite::create(
                "Auto\nOptions",
                25,
                true,
                "bigFont.fnt",
                "GJ_button_01.png",
                40.f,
                0.6f
            );
            spr->setColor({100, 100, 100});

            m_fields->m_toggleBtn = CCMenuItemSpriteExtra::create(
                spr,
                this,
                menu_selector(MyEditorUI::onToggleButton)
            );

            if (m_playtestBtn && m_playtestBtn->getParent()) {
                m_fields->m_toggleBtn->setPosition(
                    m_playtestBtn->getPosition() + ccp(70.f, 0.f)
                );
                m_playtestBtn->getParent()->addChild(m_fields->m_toggleBtn);
            }

            this->schedule(schedule_selector(MyEditorUI::visibilityUpdate));
        } else {
            m_fields->m_toggleBtn = nullptr;
        }

        return true;
    }
};