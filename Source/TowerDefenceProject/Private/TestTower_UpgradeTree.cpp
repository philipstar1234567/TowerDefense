// Fill out your copyright notice in the Description page of Project Settings.


#include "TestTower_UpgradeTree.h"

class Node
{
public:
	void Content() {};
	TArray<Node*> Children;
	void NextNode(Node*)
	{
		
	};
};

TestTower_UpgradeTree::TestTower_UpgradeTree()
{
	CreateTree();
}

void TestTower_UpgradeTree::CreateTree()
{
	Node Root;
	Node A;
	Node B;
	Node C;
	Node D;
	Node E;
	Node F;
	Node G;
	Node H;
	Node I;
	Root.Children.Append({&A, &B, &C});
	A.Children.Append({&D, &E});
	B.Children.Append({&F, &G});
	C.Children.Append({&H, &I});
}

TestTower_UpgradeTree::~TestTower_UpgradeTree()
{
}
