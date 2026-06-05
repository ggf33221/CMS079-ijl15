<p align="center">
	<a href="./README-en.md">
	English
	</a>
	/
  <a href="./README.md">
	简体中文
	</a>
</p>

⚠️ 仅供学习交流使用，未经允许禁止用于商业用途
====
鉴于某枫叶商用行为，仓库进入维护阶段，仅修复BUG，后续如有新功能更新将不再开源（2025.2.6）

适用于CMS-079版本的ijl15（<a href="./CHANGELOG.md">更新日志</a>）
====

## 使用方法
已测试的开发工具 VS 2022，SDK 10，工具集 VS2022（v144）

使用VS打开的时候注意，要使用 Release x86 MT/MD 模式生成解决方案

- MT 无需C++运行库，不支持转发方式加载 ([点击下载MT最新版](https://github.com/Willh92/CMS079-ijl15/releases/latest/download/Release-mt.zip))
- MD 需要C++运行库，支持转发方式加载 ([点击下载MD最新版](https://github.com/Willh92/CMS079-ijl15/releases/latest/download/Release-md.zip))
  
生成后可在 out相应的Release-mt/md 目录下找到 ijl15.dll

支持两种方式加载

### 覆盖方式：
覆盖源客户端的ijl15.dll,并修改配置文件中ijl15hook为false

### 转发方式 (仅支持MD模式编译文件):
源客户端的ijl15.dll重命名位2ijl15.dll，把生成的ijl15.dll拷贝到客户端目录下，ijl15hook为true

两种方式均需要把项目根目录下的config.ini复制到源客户端根目录下，修改相应配置

`新版已移除ijl15hook配置项改为自动检测`

## 主要额外添加的特殊功能

- 支持WZ与IMG模式
- 支持动态分辨率(可在系统设置中切换)
- 支持伤害皮肤以及单位显示(支持加载外置文件[Effect/DamageSkin.img](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/DamageSkin.img)或[Custom.wz](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/Custom.wz))
- 支持动态伤害皮肤加载(仅支持显示)
- 支持长键盘(支持加载外置文件[UI/QuickSlot.img](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/QuickSlot.img)或[Custom.wz](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/Custom.wz)，支持多种键位配置)
- 支持198开头的的TamingMob
- 支持高版本发型脸型
- 支持高版本武器导出直接佩戴(无需删除多余职业代码)
- 支持高版本12跟15开头的武器佩戴与定制(需要自行修改WZ匹配职业攻击,否则会攻击闪退或无法攻击)
- 支持部分inlink和outlink节点(默认使用新方法，如遇启动问题设置linkNodeNew=false)
- 支持ItemEff.img里头的特效(可用0xAA包头或ItemEffect激活)
- 支持伤害上限破功值动态修改(通过在`MapleStat`新增`DAMAGELIMIT(8388608)`枚举值,参考伤害皮肤状态实现)
- 支持内存防爆(搭配4G客户端效果更佳[MapleStory4G.exe](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/MapleStory4G.exe))
- 支持快捷二段跳（双击跳跃键）/向上二段跳（按住方向上键）
- 支持高版本宠物装备佩戴（替换佩戴校验算法）
- 支持替换装备卷轴使用算法(支持204与261开头的所有卷轴，开启时候需注意装备类型与卷轴在服务端的校验)
- 支持道具的`spec/script`节点调用脚本(效果同`reward`节点走REWARD_ITEM)
- 支持通过服务端减少装备佩戴等级（可用于修复五彩水晶等）
- 屏蔽了聊天室按钮弹窗仅发数据包（可用作解卡）
- 角色等级类型突破，角色经验突破为Long类型
- 角色面板宽度自适应（跟随背景图宽度）
- 解除坐骑模型限制，可爬绳，可二段跳，快速移动
- 修复原版SetEff.img特定条件下会导致的特效错乱


`更多特色功能请自行查看config.ini配置文件`

### 伤害皮肤说明:

`WZ模式`下伤害皮肤[Custom.wz](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/Custom.wz)放置游戏根目录下

`IMG模式`下伤害皮肤[DamageSkin.img](https://github.com/Willh92/079-ijl15/releases/download/1.0.0/DamageSkin.img)文件放在Effect下

格式为：{皮肤ID}/NoRed0   皮肤ID=0为默认皮肤 具体参考下载文件

#### 本地版本：

DamageSkin={皮肤ID}，直接开起，仅自己可见

#### 远程版本：

RemoteDamageSkin=true,服务器需要做相应的处理,主要hook了家族名称数据包做对应加载

假设GuildName为 `途插天下第一`皮肤ID为 `2`
则对应发送数据包GuildName变为`途插天下第一$$2`

GuildName不存在的时候用`#`替代变为`#$$2`

1. `client.MapleStat`添加 `DAMAGESKIN(4194304)` 枚举类型
2. `tools.MaplePacketCreator.MaplePacketCreator.updatePlayerStats`添加`DAMAGESKIN`处理为`writeInt`

#### 更新皮肤的参考代码如下

```Java
//handling.channel.handler.InterServerHandler
public static void LoggedIn(int playerid, MapleClient c) {
     //……省略
     player.sendDamageSkin();  //发送伤害皮肤包
     player.sendMacros();
      //……省略
}

//client.MapleCharacter
public void sendDamageSkin() {
    updateSingleStat(MapleStat.DAMAGESKIN, this.damageSkin);
}

//tools.MaplePacketCreator
public static byte[] updatePlayerStats(Map<MapleStat, Number> mystats, boolean itemReaction, MapleCharacter chr) {
    //……省略
                case DAMAGESKIN:
                    mplew.writeInt(statupdate2.getValue().intValue());
                    continue;
    //……省略
}

//tools.MaplePacketCreator
public void updateDamageSkin(int damageSkin) {
        this.damageSkin = damageSkin;
        MapleMap map = getMap();
        if (map != null) {
            String guildName = null;
            MapleGuild gs = World.Guild.getGuild(getGuildId());
            if (gs != null) {
                guildName = gs.getName();
            }
            if (guildName == null || guildName.length() == 0) {
                guildName = "#";
            }
            getMap().broadcastMessage(this, MaplePacketCreator.guildNameChanged(getId()
                    , guildName + "$$" + damageSkin), false);
        }
        updateSingleStat(MapleStat.DAMAGESKIN, damageSkin);
    }

public static byte[] spawnPlayerMapobject(MapleCharacter chr) {
        //……省略
        //找到对应代码块修改
        if (chr.getGuildId() <= 0) {
            mplew.writeMapleAsciiString("#$$" + chr.getDamageSkin());
            mplew.writeZeroBytes(6);
        } else {
            MapleGuild gs = World.Guild.getGuild(chr.getGuildId());
            if (gs != null) {
                mplew.writeMapleAsciiString(gs.getName() + "$$" + chr.getDamageSkin());
                mplew.writeShort(gs.getLogoBG());
                mplew.write(gs.getLogoBGColor());
                mplew.writeShort(gs.getLogo());
                mplew.write(gs.getLogoColor());
            } else {
                mplew.writeMapleAsciiString("#$$" + chr.getDamageSkin());
                mplew.writeZeroBytes(6);
            }
        }
        //……省略
```

#### 心灵控制添加角色判断

```Java
//……省略
} else if (buff.getSkill() > 0) {
//添加部分
if (buff.getStatus() == MonsterStatus.HYPNOTIZE) {
                        mplew.writeInt(-1);  //用于区分是否启用
                        MapleCharacter hypnotizeChr = buff.getHypnotizeChr();
                        mplew.writeInt(hypnotizeChr == null ? 0 : hypnotizeChr.getId());
                    }
                    //添加结束
                    mplew.writeInt((buff.getSkill() > 0) ? buff.getSkill() : 0);
}
//……省略
```

#### 通过服务端减少装备佩戴等级

```Java
//……省略
//addItemInfo内
//plew.writeInt(equip.getViciousHammer()); 原金锤子int类型拆分为两个short
mplew.writeShort(equip.getViciousHammer());
mplew.writeShort(reduceLevel); //要减少的装备佩戴等级数reduceLevel
//……省略
```

## 参考相关开源库

https://github.com/BeiDouMS/BeiDou-ijl15

https://github.com/Chronicle20/gms-83-dll

https://github.com/v3921358/MapleRoot

## 友情链接

悠悠下载器：https://github.com/uuuu233/downloader
