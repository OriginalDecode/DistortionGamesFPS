#pragma once

enum class eWeaponType
{
	PISTOL,
	SHOTGUN,
	GRENADE_LAUNCHER
};

class Weapon
{
public:
	Weapon(eWeaponType aWeaponType);
	virtual ~Weapon();

	virtual bool Shoot(const CU::Matrix44<float>& aOrientation) = 0;
	virtual void Reload() = 0;
	virtual void Update(float) {};
	const int& GetAmmoInClip() const;
	const int& GetClipSize() const;
	eWeaponType GetWeaponType();

protected:	
	int myAmmoInClip;
	int myClipSize;
	int myAmmoTotal;
	int myDamage;

	float myShootTime;
	float myShootTimer;

	eWeaponType myWeaponType;
};

inline const int& Weapon::GetAmmoInClip() const
{
	return myAmmoInClip;
}

inline const int& Weapon::GetClipSize() const
{
	return myClipSize;
}

inline eWeaponType Weapon::GetWeaponType()
{
	return myWeaponType;
}