from PyQt4.QtCore import Qt
from ert_gui.ide.wizards import TreeItem, TreeModel
from ert.test import ExtendedTestCase


class TreeStructureTest(ExtendedTestCase):

    def test_tree_item(self):

        root = TreeItem("Root")

        self.assertIsNone(root.parent())
        self.assertEqual(root.name(), "Root")
        self.assertEqual(len(root), 0)

        with self.assertRaises(IndexError):
            root.child(0)

        self.assertIsNone(root.data())

        child_1 = TreeItem("Child1")

        child = root.addChild(child_1)

        self.assertEqual(child, child_1)
        self.assertEqual(len(root), 1)
        self.assertEqual(root.child(0), child_1)
        self.assertEqual(child_1.parent(), root)

    def test_tree_model(self):
        root = TreeItem("Wizards")

        root.addChild(TreeItem("One"))

        child = root.addChild(TreeItem("Two"))
        child.addChild(TreeItem("SubOne"))
        child.addChild(TreeItem("SubTwo"))

        child_item = root.addChild(TreeItem("Three"))

        tree = TreeModel(root)

        self.assertEqual(tree.headerData(0, Qt.Horizontal), root.name())

        name1 = tree.data(tree.index(0, 0))
        self.assertEqual(name1, "One")

        name2_index = tree.index(1, 0)
        sub_name2_index = tree.index(1, 0, name2_index)

        self.assertEqual(tree.data(sub_name2_index), "SubTwo")


        tree_item = tree.item(tree.index(2, 0))

        self.assertEqual(child_item, tree_item)

        self.assertTrue(tree.data(tree.index(3, 0)).isNull())







